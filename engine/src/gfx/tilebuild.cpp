#include "tilebuild.h"

#include <chrono>
#include <cmath>

namespace ely {

namespace {
constexpr double kPi = 3.14159265358979;
}  // namespace

BuiltTile buildTile(const Terrain& terrain, const TileSpec& spec,
                    JobSystem* jobs) {
    BuiltTile out;
    out.lod = spec.lod;

    const double vsize = lodVoxelSize(spec.lod);
    const int nx = int(spec.sizeX / vsize + 0.5);
    const int ny = int(spec.sizeY / vsize + 0.5);
    const int nz = int(spec.sizeZ / vsize + 0.5);
    if (nx <= 0 || ny <= 0 || nz <= 0) return out;

    out.bounds = Aabb::fromOriginSize(spec.ox, spec.oy, spec.oz,
                                      spec.sizeX, spec.sizeY, spec.sizeZ);

    // Open sky costs nothing to skip and a full generation to discover.
    //
    // Most tiles in a resident set are above the ground. Generating one means
    // walking every column of it to be told, a thousand samples later, that it
    // is air. surfaceAltitude is the topmost solid point of a column, so a tile
    // whose floor is above the highest surface in its footprint cannot contain
    // anything — and finding that out costs a 5x5 sample of the footprint
    // instead of nx*ny*nz.
    //
    // The margin is what keeps it honest: the grid could step over a spire
    // between samples, so the test only fires when the tile floor clears the
    // highest sample by more than a tile's own width. Conservative on purpose;
    // a wrong skip is a hole in the world, and this cull is only worth having
    // if it can never make one.
    {
        const double mpuEarly = terrain.planet().radius * kPi / 4.0;
        double highest = -1e30;
        for (int gz = 0; gz <= 4; ++gz)
            for (int gx = 0; gx <= 4; ++gx) {
                const double u = spec.uCentre + (spec.ox + spec.sizeX * gx / 4.0) / mpuEarly;
                const double v = spec.vCentre + (spec.oz + spec.sizeZ * gz / 4.0) / mpuEarly;
                const double h = terrain.surfaceAltitude(spec.face, u, v);
                if (h > highest) highest = h;
            }
        if (spec.oy > highest + spec.sizeX) {
            out.skippedAsSky = true;
            return out;
        }
    }

    out.voxelsGenerated = long(nx) * ny * nz;

    // --- Generate, with a one-voxel apron on every face.
    const auto t0 = std::chrono::steady_clock::now();
    const double mpu = terrain.planet().radius * kPi / 4.0;
    const int pad = 1;
    const int ex = nx + 2 * pad, ey = ny + 2 * pad, ez = nz + 2 * pad;
    std::vector<Material> vox(size_t(ex) * ey * ez, MAT_AIR);

    // Columns are independent: each writes only its own vertical run and reads
    // a terrain that is immutable. That is what makes this safe to split with
    // no locking and no reordering — index i always writes slot i, so the
    // result cannot depend on which thread got there first.
    auto buildColumn = [&](size_t index) {
        const int az = int(index / size_t(ex));
        const int ax = int(index % size_t(ex));
        const double u = spec.uCentre + (spec.ox + (ax - pad + 0.5) * vsize) / mpu;
        const double v = spec.vCentre + (spec.oz + (az - pad + 0.5) * vsize) / mpu;
        // One column walk serves the whole vertical run: climate and surface
        // altitude are properties of the column, and hoisting them is the
        // difference between half a second and twenty milliseconds per chunk
        // (see Terrain::columnAt).
        const Terrain::Column col = terrain.columnAt(spec.face, u, v);
        for (int ay = 0; ay < ey; ++ay) {
            vox[(size_t(ay) * ez + az) * ex + ax] = terrain.materialInColumn(
                col, spec.oy + (ay - pad + 0.5) * vsize, vsize);
        }
    };

    const size_t columns = size_t(ex) * size_t(ez);
    if (jobs) {
        // A grain of 8 columns: a column is roughly five microseconds, so
        // handing over fewer than a handful costs more in coordination than it
        // saves.
        jobs->parallelFor(0, columns, buildColumn, 8);
    } else {
        for (size_t i = 0; i < columns; ++i) buildColumn(i);
    }
    const auto t1 = std::chrono::steady_clock::now();
    out.generateMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // --- Apply the player's edits over the generated terrain.
    //
    // This is the whole of the save paradigm at the point of use: generate the
    // world from the seed, then overwrite the handful of voxels somebody
    // changed. A planet nobody has touched takes this branch and does nothing.
    //
    // Edits are applied to the PADDED array, apron included. A doorway cut at
    // the very edge of a tile has to be visible to the neighbouring tile's
    // mesher too, or the two disagree about whether there is a face there and
    // the seam shows.
    if (spec.edits && !spec.edits->empty()) {
        // The tile's extent in the face's own global metre grid — not the local
        // frame, which is centred whereever this session happened to start.
        const double gx0 = spec.uCentre * mpu + spec.ox - vsize;
        const double gy0 = spec.oy - vsize;
        const double gz0 = spec.vCentre * mpu + spec.oz - vsize;
        const double gx1 = gx0 + double(ex) * vsize;
        const double gy1 = gy0 + double(ey) * vsize;
        const double gz1 = gz0 + double(ez) * vsize;

        const ChunkAddress lo =
            ChunkAddress::fromMetres(uint8_t(spec.face), gx0, gy0, gz0);
        const ChunkAddress hi =
            ChunkAddress::fromMetres(uint8_t(spec.face), gx1, gy1, gz1);

        // Walk the chunks the tile overlaps and ask each for its edits, rather
        // than asking per voxel. Almost every chunk answers "none" in one hash
        // lookup, so an untouched region costs a handful of misses for the
        // whole tile instead of one per voxel.
        for (int32_t cy = lo.y; cy <= hi.y; ++cy)
        for (int32_t cz = lo.z; cz <= hi.z; ++cz)
        for (int32_t cx = lo.x; cx <= hi.x; ++cx) {
            const ChunkAddress chunk{uint8_t(spec.face), cx, cy, cz};
            const EditStore::ChunkEdits* edits = spec.edits->chunkEdits(chunk);
            if (!edits) continue;

            for (const auto& entry : *edits) {
                const VoxelAddress addr{entry.first};
                const int bi = addr.blockIndex();
                const int bx = bi % kChunkSize;
                const int bz = (bi / kChunkSize) % kChunkSize;
                const int by = bi / (kChunkSize * kChunkSize);

                // Centre of the edited voxel, in global face metres.
                double wx = double(cx) * kChunkSize + bx;
                double wy = double(cy) * kChunkSize + by;
                double wz = double(cz) * kChunkSize + bz;
                double extent = 1.0;
                if (addr.isMicro()) {
                    const int mi = addr.microIndex();
                    wx += (mi % kMicro) * kMicroSize;
                    wz += ((mi / kMicro) % kMicro) * kMicroSize;
                    wy += (mi / (kMicro * kMicro)) * kMicroSize;
                    extent = kMicroSize;
                }
                // A voxel coarser than the edit cannot represent it. Rather than
                // silently dropping the edit, the edit claims the voxel it falls
                // inside: a doorway seen from 200 m away is a smudge, but a
                // smudge in the right place beats a wall that is not there when
                // you walk up to it.
                (void)extent;
                wx += 0.5 * (addr.isMicro() ? kMicroSize : 1.0);
                wy += 0.5 * (addr.isMicro() ? kMicroSize : 1.0);
                wz += 0.5 * (addr.isMicro() ? kMicroSize : 1.0);

                const int ax = int(std::floor((wx - gx0) / vsize));
                const int ay = int(std::floor((wy - gy0) / vsize));
                const int az = int(std::floor((wz - gz0) / vsize));
                if (ax < 0 || ay < 0 || az < 0 || ax >= ex || ay >= ey || az >= ez)
                    continue;
                vox[(size_t(ay) * ez + az) * ex + ax] = Material(entry.second);
            }
        }
    }

    // Logical coordinates run 0..n-1; -1 and n reach into the apron, which is
    // exactly what the mesher asks about when deciding whether a boundary face
    // is exposed.
    auto sample = [&](int x, int y, int z) -> Material {
        if (spec.clipToScene) {
            const double wx = spec.ox + (x + 0.5) * vsize;
            const double wy = spec.oy + (y + 0.5) * vsize;
            const double wz = spec.oz + (z + 0.5) * vsize;
            if (wx < spec.sceneMinX || wx > spec.sceneMaxX ||
                wy < spec.sceneMinY || wy > spec.sceneMaxY ||
                wz < spec.sceneMinZ || wz > spec.sceneMaxZ)
                return MAT_AIR;
        }
        const int ax = x + pad, ay = y + pad, az = z + pad;
        if (ax < 0 || ay < 0 || az < 0 || ax >= ex || ay >= ey || az >= ez)
            return MAT_AIR;
        return vox[(size_t(ay) * ez + az) * ex + ax];
    };

    // --- Mesh.
    const auto t2 = std::chrono::steady_clock::now();
    VisibilityMask visible(nx, ny, nz, sample);

    MeshOptions opt;
    opt.voxelSize = vsize;
    opt.ox = spec.ox;
    opt.oy = spec.oy;
    opt.oz = spec.oz;
    opt.visibility = &visible;
    // Back-face culling is deliberately NOT enabled here. It depends on the
    // camera, and a tile is built once and drawn from wherever the player
    // walks — baking a view direction into the geometry would mean remeshing
    // every time they turn around. The GPU culls back faces per frame for
    // free; this cull only earns its keep in the offline tools, where the
    // camera really is fixed.
    Mesh mesh = greedyMesh(nx, ny, nz, sample, opt, &out.mesh);
    const auto t3 = std::chrono::steady_clock::now();
    out.meshMs = std::chrono::duration<double, std::milli>(t3 - t2).count();

    out.gpu = toGpuMesh(mesh, spec.ox, spec.oy, spec.oz, vsize);
    return out;
}

}  // namespace ely

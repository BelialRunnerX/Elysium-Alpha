// Saving: only what the player changed.
//
// The world is a pure function of its seed. Every voxel of every planet in the
// galaxy can be recomputed from a 64-bit number, which means storing the world
// is not merely wasteful — it is storing the answer to a question the generator
// already answers for free. What cannot be recomputed is what a player *did*,
// and that is all this stores.
//
// The consequence is the property asked for: visit a planet, cut a doorway into
// a cliff, fly to another system, come back years later, and the doorway is
// there — while the untouched 99.999% of that planet costs nothing at all,
// because it was never saved. A save file grows with what a player has done,
// not with where they have been.
//
// THE FORMAT is an append-only journal per planet.
//
//   [header] [record] [record] [record] ...
//
// Appending is the whole design. A player mining is a stream of small edits; a
// format that had to seek, read-modify-write and re-index for each one would
// either stall the game or need a write-behind cache with its own consistency
// problems. Appending is one write at the end of a file. Loading replays the
// journal into memory, later records winning over earlier ones, so a voxel
// edited five times costs five records on disk and one entry in memory until
// the next compaction.
//
// CRASH SAFETY falls out of the same choice. A process killed mid-write leaves
// a torn record at the end and nothing else damaged, so loading stops at the
// first record that fails its checksum and keeps everything before it. The
// player loses the last edit rather than the planet. Every record carries a
// CRC32 for exactly this: a short read is detectable, but a *plausible* torn
// record is not, and that is the one that would corrupt a world silently.
//
// THE GENERATOR FINGERPRINT in the header is the other half of the contract.
// Edits are deltas against generated terrain, so they only mean anything
// against the generator that produced that terrain. If the noise changes, the
// stored edits are still perfectly valid data describing changes to a world
// that no longer exists — a doorway now hanging in mid-air, or buried. The
// fingerprint makes that detectable instead of silent, which is principle E7:
// a silent failure is the worse bug.
#pragma once
#include "../world/material.h"
#include "../world/resolution.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ely {

// Where a chunk is, globally and permanently.
//
// Deliberately NOT the local metre frame the tile builder works in — that frame
// is centred on wherever a tool or a player happened to start, and an address
// that depends on that is an address that means something different tomorrow.
// This is the face of the cube-sphere and the chunk's integer position in that
// face's own metre grid, which is a property of the planet and nothing else.
struct ChunkAddress {
    uint8_t face = 0;            // 0..5, the cube-sphere face
    int32_t x = 0, y = 0, z = 0; // chunk indices; y is altitude

    bool operator==(const ChunkAddress& o) const {
        return face == o.face && x == o.x && y == o.y && z == o.z;
    }

    // The chunk containing a point given in that face's metre grid.
    static ChunkAddress fromMetres(uint8_t face, double x, double y, double z);
};

struct ChunkAddressHash {
    size_t operator()(const ChunkAddress& a) const;
};

// Which voxel inside a chunk, at either resolution.
//
// One 32-bit word covers both, because an edit journal is mostly these and
// eight bytes per edit against twelve is a third of the file:
//
//   bits 0-14    block index within the chunk    (32^3 = 32768 needs 15)
//   bits 15-27   micro index within that block, plus one; 0 means the whole
//                block                            (16^3 = 4096 needs 13 with
//                                                  the +1)
struct VoxelAddress {
    uint32_t bits = 0;

    static VoxelAddress wholeBlock(int x, int y, int z) {
        VoxelAddress a;
        a.bits = uint32_t(blockIndexOf(x, y, z));
        return a;
    }
    static VoxelAddress microVoxel(int x, int y, int z, int mx, int my, int mz) {
        VoxelAddress a;
        const uint32_t micro = uint32_t((my * kMicro + mz) * kMicro + mx);
        a.bits = uint32_t(blockIndexOf(x, y, z)) | ((micro + 1u) << 15);
        return a;
    }

    bool isMicro() const { return (bits >> 15) != 0; }
    int blockIndex() const { return int(bits & 0x7FFF); }
    int microIndex() const { return int((bits >> 15) - 1u); }   // only if isMicro

    static int blockIndexOf(int x, int y, int z) {
        return (y * kChunkSize + z) * kChunkSize + x;
    }
};

// What a load found, so a caller can say something useful rather than guess.
struct LoadReport {
    bool fileExisted = false;
    bool headerValid = false;
    bool fingerprintMatches = true;
    uint64_t recordsRead = 0;
    uint64_t editsApplied = 0;
    uint64_t bytesRead = 0;
    bool truncatedTail = false;   // a torn record was discarded
    std::string message;
};

class EditStore {
public:
    EditStore() = default;

    // --- The world's deltas -------------------------------------------------

    // Record an edit. Later edits to the same voxel replace earlier ones in
    // memory; on disk both are present until compaction, which is the price of
    // an append-only journal and a good trade.
    void setBlock(const ChunkAddress& chunk, int x, int y, int z, Material m);
    void setMicro(const ChunkAddress& chunk, int x, int y, int z,
                  int mx, int my, int mz, Material m);

    // Look an edit up. Returns false when the generator's answer stands, which
    // is the overwhelmingly common case and is why this is a hash lookup that
    // usually misses rather than a per-voxel array that always allocates.
    bool lookupBlock(const ChunkAddress& chunk, int x, int y, int z,
                     Material& out) const;
    bool lookupMicro(const ChunkAddress& chunk, int x, int y, int z,
                     int mx, int my, int mz, Material& out) const;

    // The edits in one chunk, or null. Callers that are about to walk a whole
    // chunk should take this once rather than looking up per voxel.
    using ChunkEdits = std::unordered_map<uint32_t, uint16_t>;
    const ChunkEdits* chunkEdits(const ChunkAddress& chunk) const;

    bool empty() const { return chunks_.empty(); }
    size_t editedChunkCount() const { return chunks_.size(); }
    size_t editCount() const;
    size_t pendingCount() const { return pending_.size(); }

    // --- Persistence --------------------------------------------------------

    // Replay a journal. Missing file is success with an empty store: a planet
    // nobody has touched has no file, and that is not an error.
    LoadReport load(const std::string& path, uint64_t planetSeed,
                    uint64_t generatorFingerprint);

    // Append everything recorded since the last flush. Cheap and safe to call
    // often — that is what an append-only journal is for.
    bool flush(std::string* error = nullptr);

    // Rewrite the file with the merged state, dropping superseded records.
    // Written to a temporary and renamed, so a crash during compaction leaves
    // the original journal intact.
    bool compact(std::string* error = nullptr);

    // Compaction is worth doing when the journal holds substantially more
    // records than there are live edits.
    bool shouldCompact() const;

    const std::string& path() const { return path_; }
    uint64_t planetSeed() const { return planetSeed_; }
    uint64_t generatorFingerprint() const { return fingerprint_; }

private:
    std::unordered_map<ChunkAddress, ChunkEdits, ChunkAddressHash> chunks_;

    // Edits not yet on disk, in the order they were made. Order matters: replay
    // takes the last write, so writing them out of order would resurrect an
    // edit the player has already undone.
    struct Pending { ChunkAddress chunk; uint32_t addr; uint16_t material; };
    std::vector<Pending> pending_;

    std::string path_;
    uint64_t planetSeed_ = 0;
    uint64_t fingerprint_ = 0;
    uint64_t recordsOnDisk_ = 0;

    void remember(const ChunkAddress& chunk, uint32_t addr, Material m);
};

}  // namespace ely

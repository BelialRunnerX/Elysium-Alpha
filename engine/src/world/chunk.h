// Chunk storage. Specification §12.1, extended with sub-block detail.
//
// A chunk is 32^3 *blocks* of one metre. Every block may, optionally, carry a
// 16^3 volume of micro-voxels — 4096 subunits of 62.5 mm — but almost none of
// them do, and that is the whole design:
//
//   * The block grid is a PalettedVolume<32>. Homogeneous chunks store a
//     single material and no array at all, which is the case that matters
//     most: a 3 km planet is roughly 130,000 chunk columns of 24 layers, and
//     the great majority of those layers are uniformly stone or uniformly air.
//
//   * Micro-detail is a *sparse map*, block index -> PalettedVolume<16>,
//     materialised only when something actually needs sub-block shape. An
//     untouched chunk holds zero micro volumes, so micro-voxels cost nothing
//     until they are used.
//
// The reason sparseness is enough — rather than a desperate compromise — is
// that the generator is a pure function of position (see resolution.h). Terrain
// that has never been edited can be *sampled* at 62.5 mm whenever the camera is
// close enough to care, straight from the noise fields, with no storage at all.
// Stored micro-voxels are therefore only ever needed for deviations from the
// generator: a player's cuts and fills, and placed structures. Storage tracks
// what the player did, not what the world is.
//
// Dense-everywhere micro storage would be 4096x the memory of the block grid —
// tens of gigabytes for the loaded set. Sparse-on-edit is a few megabytes for a
// heavily worked base. That factor is the reason this file looks the way it
// does.
#pragma once
#include "resolution.h"
#include "volume.h"
#include <cstdint>
#include <unordered_map>
#include <utility>

namespace ely {

inline int chunkIndex(int x, int y, int z) {
    return (y * kChunkSize + z) * kChunkSize + x;
}

using BlockVolume = PalettedVolume<kChunkSize>;   // 32^3 one-metre blocks
using MicroVolume = PalettedVolume<kMicro>;       // 16^3 subunits of one block

class Chunk {
public:
    // --- Block level (one metre) -------------------------------------------

    // A chunk is homogeneous only if it has no detail volumes either: a single
    // carved block makes the chunk non-trivial even if every block material is
    // still stone.
    bool homogeneous() const { return blocks_.homogeneous() && detail_.empty(); }
    Material uniform() const { return blocks_.uniform(); }
    int paletteSize() const { return blocks_.paletteSize(); }

    Material get(int x, int y, int z) const { return blocks_.get(x, y, z); }

    // Writing a whole block discards any sub-block detail it had. Placing a
    // solid metre of stone over a carved block should not leave the carving
    // hiding underneath, ready to reappear.
    void set(int x, int y, int z, Material m) {
        blocks_.set(x, y, z, m);
        detail_.erase(key(x, y, z));
    }

    template <typename Fn>
    void fill(Fn&& sample) {
        detail_.clear();
        blocks_.fill(std::forward<Fn>(sample));
    }

    // --- Micro level (62.5 mm) ---------------------------------------------

    bool hasDetail(int x, int y, int z) const {
        return detail_.find(key(x, y, z)) != detail_.end();
    }

    const MicroVolume* detail(int x, int y, int z) const {
        auto it = detail_.find(key(x, y, z));
        return it == detail_.end() ? nullptr : &it->second;
    }

    // The material of one micro-voxel. Blocks without detail answer with their
    // own material, so a caller can walk micro-voxels uniformly and never has
    // to ask whether detail exists.
    Material micro(int x, int y, int z, int mx, int my, int mz) const {
        auto it = detail_.find(key(x, y, z));
        if (it == detail_.end()) return blocks_.get(x, y, z);
        return it->second.get(mx, my, mz);
    }

    void setMicro(int x, int y, int z, int mx, int my, int mz, Material m) {
        MicroVolume& v = materialise(x, y, z);
        v.set(mx, my, mz, m);
        collapse(x, y, z, v);
    }

    // Fill a block's detail from a sampler over its 16^3 subunits. If the
    // result is uniform the detail volume is dropped and the block simply
    // becomes that material — the common outcome deep underground, and the
    // reason sampling at micro resolution does not explode storage.
    template <typename Fn>
    void fillDetail(int x, int y, int z, Fn&& sample) {
        MicroVolume& v = materialise(x, y, z);
        v.fill(std::forward<Fn>(sample));
        collapse(x, y, z, v);
    }

    int detailBlocks() const { return int(detail_.size()); }

    size_t bytes() const {
        size_t n = sizeof(Chunk) + blocks_.bytes() - sizeof(BlockVolume);
        for (const auto& kv : detail_)
            n += sizeof(uint16_t) + kv.second.bytes();
        return n;
    }

private:
    static uint16_t key(int x, int y, int z) {
        return uint16_t(chunkIndex(x, y, z));
    }

    MicroVolume& materialise(int x, int y, int z) {
        const uint16_t k = key(x, y, z);
        auto it = detail_.find(k);
        if (it != detail_.end()) return it->second;
        // A new detail volume starts as 4096 copies of the block it replaces,
        // so materialising is never a visible change — only an edit is.
        return detail_.emplace(k, MicroVolume(blocks_.get(x, y, z))).first->second;
    }

    void collapse(int x, int y, int z, MicroVolume& v) {
        if (!v.compact()) return;
        blocks_.set(x, y, z, v.uniform());
        detail_.erase(key(x, y, z));
    }

    BlockVolume blocks_;
    std::unordered_map<uint16_t, MicroVolume> detail_;
};

}  // namespace ely

// A palette-compressed cube of voxels, at any edge length.
//
// Used at two scales, which is the point of making it a template:
//
//   PalettedVolume<32>  a chunk of 1 m blocks
//   PalettedVolume<16>  the micro-voxels inside one block
//
// Both want exactly the same three properties, and the first edition's lesson
// about one source of truth applies to code as much as to data: two copies of
// this would drift, and the bug would be a corrupted world.
//
//   * Homogeneous volumes store a single material and no array at all. Most of
//     a planet is solid stone or empty air, and at micro scale that is even
//     more true — the overwhelming majority of blocks are entirely one thing.
//   * A palette plus packed indices, 4 bits while the palette fits in 16
//     entries, widening to 8 and 16 as needed.
//   * No allocation until something is actually mixed.
#pragma once
#include "material.h"
#include <cstdint>
#include <vector>

namespace ely {

template <int EDGE>
class PalettedVolume {
public:
    static constexpr int kEdge = EDGE;
    static constexpr int kVolume = EDGE * EDGE * EDGE;

    static int index(int x, int y, int z) { return (y * EDGE + z) * EDGE + x; }

    PalettedVolume() : uniform_(MAT_AIR), bits_(0), homogeneous_(true) {}
    explicit PalettedVolume(Material m)
        : uniform_(m), bits_(0), homogeneous_(true) {}

    bool homogeneous() const { return homogeneous_; }
    Material uniform() const { return uniform_; }
    int paletteSize() const { return int(palette_.size()); }

    Material get(int x, int y, int z) const {
        if (homogeneous_) return uniform_;
        return palette_[readIndex(index(x, y, z))];
    }

    void set(int x, int y, int z, Material m) {
        if (homogeneous_) {
            if (m == uniform_) return;
            expand();
        }
        writeIndex(index(x, y, z), paletteIndex(m));
    }

    template <typename Fn>
    void fill(Fn&& sample) {
        Material first = sample(0, 0, 0);
        bool same = true;
        std::vector<Material> tmp(kVolume);
        for (int y = 0; y < EDGE; ++y)
            for (int z = 0; z < EDGE; ++z)
                for (int x = 0; x < EDGE; ++x) {
                    Material m = sample(x, y, z);
                    tmp[index(x, y, z)] = m;
                    if (m != first) same = false;
                }
        if (same) {
            homogeneous_ = true; uniform_ = first;
            data_.clear(); data_.shrink_to_fit();
            palette_.clear(); palette_.shrink_to_fit();
            bits_ = 0;
            return;
        }
        homogeneous_ = false;
        palette_.clear();
        bits_ = 4;
        data_.assign(size_t(kVolume) * 4 / 8, 0);
        for (int i = 0; i < kVolume; ++i) writeIndex(i, paletteIndex(tmp[i]));
    }

    // True if every voxel is non-solid, which lets a mesher skip the whole
    // volume without reading it.
    bool empty() const { return homogeneous_ && !isSolid(uniform_); }
    bool full() const { return homogeneous_ && isSolid(uniform_); }

    // Collapse back to the homogeneous form if editing has left the volume
    // uniform again.
    //
    // This is not cosmetic. A player who carves a block down to micro-voxels
    // and then fills it back in must not leave a 2 KB allocation behind for
    // every such block, or a long session leaks storage in proportion to how
    // much the player has fiddled rather than to how much they have changed.
    // Callers do this after edits; fill() already handles its own case.
    bool compact() {
        if (homogeneous_) return true;
        const Material first = palette_[readIndex(0)];
        for (int i = 1; i < kVolume; ++i)
            if (palette_[readIndex(i)] != first) return false;
        homogeneous_ = true;
        uniform_ = first;
        bits_ = 0;
        data_.clear(); data_.shrink_to_fit();
        palette_.clear(); palette_.shrink_to_fit();
        return true;
    }

    size_t bytes() const {
        return sizeof(*this) + data_.capacity()
             + palette_.capacity() * sizeof(Material);
    }

private:
    void expand() {
        homogeneous_ = false;
        palette_.clear();
        bits_ = 4;
        data_.assign(size_t(kVolume) * 4 / 8, 0);
        const uint32_t p = paletteIndex(uniform_);
        for (int i = 0; i < kVolume; ++i) writeIndex(i, p);
    }

    uint32_t paletteIndex(Material m) {
        for (size_t i = 0; i < palette_.size(); ++i)
            if (palette_[i] == m) return uint32_t(i);
        palette_.push_back(m);
        const uint32_t idx = uint32_t(palette_.size() - 1);
        if (idx >= (1u << bits_)) widen();
        return idx;
    }

    void widen() {
        const int oldBits = bits_;
        const int newBits = (oldBits == 4) ? 8 : 16;
        const std::vector<uint8_t> old = data_;
        data_.assign(size_t(kVolume) * size_t(newBits) / 8, 0);
        bits_ = newBits;
        for (int i = 0; i < kVolume; ++i) {
            uint32_t v;
            if (oldBits == 4) {
                const uint8_t byte = old[size_t(i) >> 1];
                v = (i & 1) ? uint32_t(byte >> 4) : uint32_t(byte & 0x0F);
            } else {
                v = old[size_t(i)];
            }
            writeIndex(i, v);
        }
    }

    uint32_t readIndex(int i) const {
        if (bits_ == 4) {
            const uint8_t byte = data_[size_t(i) >> 1];
            return (i & 1) ? uint32_t(byte >> 4) : uint32_t(byte & 0x0F);
        }
        if (bits_ == 8) return data_[size_t(i)];
        return uint32_t(data_[size_t(i) * 2])
             | (uint32_t(data_[size_t(i) * 2 + 1]) << 8);
    }

    void writeIndex(int i, uint32_t v) {
        if (bits_ == 4) {
            uint8_t& byte = data_[size_t(i) >> 1];
            byte = (i & 1) ? uint8_t((byte & 0x0F) | ((v & 0x0F) << 4))
                           : uint8_t((byte & 0xF0) | (v & 0x0F));
        } else if (bits_ == 8) {
            data_[size_t(i)] = uint8_t(v);
        } else {
            data_[size_t(i) * 2]     = uint8_t(v & 0xFF);
            data_[size_t(i) * 2 + 1] = uint8_t((v >> 8) & 0xFF);
        }
    }

    Material uniform_;
    int bits_;
    bool homogeneous_;
    std::vector<Material> palette_;
    std::vector<uint8_t> data_;
};

}  // namespace ely

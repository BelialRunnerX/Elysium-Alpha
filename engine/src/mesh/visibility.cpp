#include "visibility.h"
#include <vector>

namespace ely {

VisibilityMask::VisibilityMask(int sx, int sy, int sz, const VoxelSampler& sample)
    : sx_(sx), sy_(sy), sz_(sz),
      reachedBits_(size_t(sx) * size_t(sy) * size_t(sz), 0) {
    // One pass to classify, so the sampler — which may be the generator — is
    // called exactly once per voxel. On this project that call is the single
    // most expensive thing in the pipeline, so nothing here may call it twice.
    std::vector<uint8_t> solid(reachedBits_.size(), 0);
    for (int y = 0; y < sy; ++y)
        for (int z = 0; z < sz; ++z)
            for (int x = 0; x < sx; ++x) {
                const bool s = isSolid(sample(x, y, z));
                solid[size_t(idx(x, y, z))] = s ? 1 : 0;
                if (s) ++solid_; else ++air_;
            }

    // Flood fill from every non-solid voxel on the region boundary. An
    // explicit stack rather than recursion: a 256^3 region is 16 M voxels and
    // a connected cave system can be most of them.
    std::vector<int> stack;
    stack.reserve(size_t(sx) * size_t(sz) * 4);

    auto push = [&](int x, int y, int z) {
        const int i = idx(x, y, z);
        if (solid[size_t(i)] || reachedBits_[size_t(i)]) return;
        reachedBits_[size_t(i)] = 1;
        ++reached_;
        stack.push_back(i);
    };

    for (int y = 0; y < sy; ++y)
        for (int z = 0; z < sz; ++z)
            for (int x = 0; x < sx; ++x) {
                const bool boundary = x == 0 || y == 0 || z == 0 ||
                                      x == sx - 1 || y == sy - 1 || z == sz - 1;
                if (boundary) push(x, y, z);
            }

    while (!stack.empty()) {
        const int i = stack.back();
        stack.pop_back();
        const int x = i % sx_;
        const int z = (i / sx_) % sz_;
        const int y = i / (sx_ * sz_);
        if (x > 0)       push(x - 1, y, z);
        if (x < sx_ - 1) push(x + 1, y, z);
        if (y > 0)       push(x, y - 1, z);
        if (y < sy_ - 1) push(x, y + 1, z);
        if (z > 0)       push(x, y, z - 1);
        if (z < sz_ - 1) push(x, y, z + 1);
    }
}

}  // namespace ely

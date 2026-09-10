#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Describe microvoxel material shading inputs, damage masks, emissive behavior, transparency, and biome tint hooks.
struct VoxelMaterialRendererCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct VoxelMaterialRendererRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class VoxelMaterialRendererService {
public:
    bool submit(const VoxelMaterialRendererCommand& command);
    const VoxelMaterialRendererRecord* lookup(std::uint64_t subjectId) const;
    std::vector<VoxelMaterialRendererRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, VoxelMaterialRendererRecord> records_;
};

}

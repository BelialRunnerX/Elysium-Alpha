// Intended function: Schedule chunk mesh builds/rebuilds by LOD, visibility, edit urgency, worker budget, and stale-revision rejection.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::render {
struct MeshBuildRequest {
    std::uint64_t requestId{};
    std::uint64_t chunkKey{};
    std::uint64_t lodTier{};
    std::uint64_t priority{};
    std::uint64_t revision{};
    double estimatedCost{};
};
class MeshBuildRequestRegistry {
public:
    bool publish(MeshBuildRequest record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const MeshBuildRequest* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<MeshBuildRequest>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const MeshBuildRequest& r) noexcept;
    std::vector<MeshBuildRequest> records_;
};
} // namespace elysium::render

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Apply ordered data-driven patches to registries while preserving immutable stable IDs after registry freeze.
struct DataPatchSystemRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct DataPatchSystemState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class DataPatchSystemSystem {
public:
    bool apply(const DataPatchSystemRequest& request);
    bool erase(std::uint64_t targetId);
    const DataPatchSystemState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, DataPatchSystemState> states_;
};

} // namespace elysium

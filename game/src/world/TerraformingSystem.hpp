#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Apply bounded authored terrain/ecology/atmosphere transformation requests as persistent chunk deltas.
struct TerraformingSystemRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct TerraformingSystemState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class TerraformingSystemSystem {
public:
    bool apply(const TerraformingSystemRequest& request);
    bool erase(std::uint64_t targetId);
    const TerraformingSystemState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, TerraformingSystemState> states_;
};

} // namespace elysium

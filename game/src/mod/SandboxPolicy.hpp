#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Describe capability limits for future scripts so mods cannot directly mutate authoritative simulation stores.
struct SandboxPolicyRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct SandboxPolicyState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class SandboxPolicySystem {
public:
    bool apply(const SandboxPolicyRequest& request);
    bool erase(std::uint64_t targetId);
    const SandboxPolicyState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, SandboxPolicyState> states_;
};

} // namespace elysium

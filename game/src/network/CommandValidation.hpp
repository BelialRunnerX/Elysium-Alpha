#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Validate future network gameplay commands against identity, ownership, permissions, sequence, and replay rules.
struct CommandValidationRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct CommandValidationState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class CommandValidationSystem {
public:
    bool apply(const CommandValidationRequest& request);
    bool erase(std::uint64_t targetId);
    const CommandValidationState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, CommandValidationState> states_;
};

} // namespace elysium

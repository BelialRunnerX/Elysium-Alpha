#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate context-aware contracts and quest chains from settlement needs, threats, discoveries, and faction goals.
struct DynamicQuestGeneratorRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct DynamicQuestGeneratorState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class DynamicQuestGeneratorSystem {
public:
    bool apply(const DynamicQuestGeneratorRequest& request);
    bool erase(std::uint64_t targetId);
    const DynamicQuestGeneratorState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, DynamicQuestGeneratorState> states_;
};

} // namespace elysium

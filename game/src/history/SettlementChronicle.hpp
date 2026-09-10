#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Summarize settlement founding, disasters, leaders, expansions, conflicts, migrations, and major achievements.
struct SettlementChronicleRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct SettlementChronicleState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class SettlementChronicleSystem {
public:
    bool apply(const SettlementChronicleRequest& request);
    bool erase(std::uint64_t targetId);
    const SettlementChronicleState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, SettlementChronicleState> states_;
};

} // namespace elysium

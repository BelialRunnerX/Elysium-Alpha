#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate faction splinters, agendas, leaders, relationships, and operational doctrines from civilization context.
struct FactionGeneratorRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct FactionGeneratorState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class FactionGeneratorSystem {
public:
    bool apply(const FactionGeneratorRequest& request);
    bool erase(std::uint64_t targetId);
    const FactionGeneratorState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, FactionGeneratorState> states_;
};

} // namespace elysium

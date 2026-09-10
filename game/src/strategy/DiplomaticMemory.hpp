#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Store bounded relationship memories that influence future negotiations without retaining full event history.
struct DiplomaticMemoryRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct DiplomaticMemoryState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class DiplomaticMemorySystem {
public:
    bool apply(const DiplomaticMemoryRequest& request);
    bool erase(std::uint64_t targetId);
    const DiplomaticMemoryState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, DiplomaticMemoryState> states_;
};

} // namespace elysium

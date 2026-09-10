#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Record strategic wars, belligerents, objectives, decisive events, territorial outcomes, and long-term memories.
struct WarHistoryRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct WarHistoryState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class WarHistorySystem {
public:
    bool apply(const WarHistoryRequest& request);
    bool erase(std::uint64_t targetId);
    const WarHistoryState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, WarHistoryState> states_;
};

} // namespace elysium

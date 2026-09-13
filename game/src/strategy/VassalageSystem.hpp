#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent asymmetric protectorate and vassal relations, tribute, defense obligations, and autonomy.
struct VassalageSystemRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct VassalageSystemState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class VassalageSystemSystem {
public:
    bool apply(const VassalageSystemRequest& request);
    bool erase(std::uint64_t targetId);
    const VassalageSystemState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, VassalageSystemState> states_;
};

} // namespace elysium

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Summarize solar storms, debris, radiation, piracy, and anomaly exposure for strategic travel planning.
struct SystemHazardModelRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct SystemHazardModelState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class SystemHazardModelSystem {
public:
    bool apply(const SystemHazardModelRequest& request);
    bool erase(std::uint64_t targetId);
    const SystemHazardModelState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, SystemHazardModelState> states_;
};

} // namespace elysium

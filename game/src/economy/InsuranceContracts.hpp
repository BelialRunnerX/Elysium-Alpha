#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track deterministic cargo, facility, and expedition risk coverage plus claim settlement requests.
struct InsuranceContractsRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct InsuranceContractsState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class InsuranceContractsSystem {
public:
    bool apply(const InsuranceContractsRequest& request);
    bool erase(std::uint64_t targetId);
    const InsuranceContractsState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, InsuranceContractsState> states_;
};

} // namespace elysium

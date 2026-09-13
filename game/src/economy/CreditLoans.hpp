#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Model principal, interest, collateral references, repayment schedules, and default state using durable identities.
struct CreditLoansRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct CreditLoansState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class CreditLoansSystem {
public:
    bool apply(const CreditLoansRequest& request);
    bool erase(std::uint64_t targetId);
    const CreditLoansState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, CreditLoansState> states_;
};

} // namespace elysium

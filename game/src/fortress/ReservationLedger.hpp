#pragma once

#include "fortress/Systems.hpp"

#include <unordered_map>
#include <vector>

namespace elysium::fortress {

struct ReservationDecision {
    bool accepted{};
    std::string reason;
    std::vector<ReservationClaim> conflicts;
};

class ReservationLedger {
public:
    ReservationDecision tryClaim(ReservationClaim claim);
    std::vector<ReservationClaim> releaseJob(JobId job, std::string reason = {});
    std::vector<ReservationClaim> invalidateResource(StableId resource, std::string reason);
    std::vector<ReservationClaim> expire(std::int64_t tick);
    [[nodiscard]] std::span<const ReservationClaim> claims() const noexcept { return claims_; }

private:
    std::vector<ReservationClaim> claims_;
};

} // namespace elysium::fortress

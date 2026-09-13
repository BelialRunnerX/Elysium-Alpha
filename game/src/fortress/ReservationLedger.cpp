#include "fortress/ReservationLedger.hpp"

#include <algorithm>

namespace elysium::fortress {

ReservationDecision ReservationLedger::tryClaim(ReservationClaim claim) {
    ReservationDecision decision{true, "reservation accepted", {}};
    for (const auto& existing : claims_) {
        if (existing.resource != claim.resource) continue;
        if (!reservationCompatible(existing, claim)) {
            decision.accepted = false;
            decision.reason = "resource already has incompatible reservation";
            decision.conflicts.push_back(existing);
        }
    }
    if (!decision.accepted) return decision;
    claims_.push_back(std::move(claim));
    std::stable_sort(claims_.begin(), claims_.end(), [](const ReservationClaim& a, const ReservationClaim& b) {
        if (a.resource != b.resource) return a.resource.value < b.resource.value;
        if (a.job != b.job) return a.job.value < b.job.value;
        return static_cast<std::uint8_t>(a.kind) < static_cast<std::uint8_t>(b.kind);
    });
    return decision;
}

std::vector<ReservationClaim> ReservationLedger::releaseJob(JobId job, std::string reason) {
    std::vector<ReservationClaim> released;
    auto it = claims_.begin();
    while (it != claims_.end()) {
        if (it->job != job) {
            ++it;
            continue;
        }
        it->cancellationReason = reason;
        released.push_back(*it);
        it = claims_.erase(it);
    }
    return released;
}

std::vector<ReservationClaim> ReservationLedger::invalidateResource(StableId resource, std::string reason) {
    std::vector<ReservationClaim> released;
    auto it = claims_.begin();
    while (it != claims_.end()) {
        if (it->resource != resource) {
            ++it;
            continue;
        }
        it->cancellationReason = reason;
        released.push_back(*it);
        it = claims_.erase(it);
    }
    return released;
}

std::vector<ReservationClaim> ReservationLedger::expire(std::int64_t tick) {
    std::vector<ReservationClaim> expired;
    auto it = claims_.begin();
    while (it != claims_.end()) {
        if (it->expiresTick <= 0 || it->expiresTick > tick) {
            ++it;
            continue;
        }
        it->cancellationReason = "reservation expired";
        expired.push_back(*it);
        it = claims_.erase(it);
    }
    return expired;
}

} // namespace elysium::fortress

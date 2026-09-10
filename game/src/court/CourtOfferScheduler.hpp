#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace elysium::court {

enum class EligibilityMeter : std::uint8_t { Favor, SystemSuspicion, Always };
enum class EnvoyKind : std::uint8_t { Elysomnion, SylpharaVoss, Sentinel, Lillith, Aurelia };
enum class CourtOfferKind : std::uint8_t { Exchange, ConstructionCommission };
enum class CourtOfferState : std::uint8_t { Visible, Fulfilled, Withdrawn };
enum class CourtVisitState : std::uint8_t { Visible, Suspended, Completed, Expired, Cancelled };

struct EnvoyDefinition {
    std::uint64_t envoyId{};
    EnvoyKind kind{EnvoyKind::Sentinel};
    std::uint64_t officeId{};
    std::uint64_t offerTableId{};
    EligibilityMeter meter{EligibilityMeter::Favor};
    double minimum{};
    double maximum{100.0};
    std::uint64_t considerationIntervalTicks{6000};
    std::uint64_t visitDurationTicks{24000};
};

struct OfferDefinition {
    std::uint64_t definitionId{};
    CourtOfferKind kind{CourtOfferKind::Exchange};
    std::uint64_t rewardRef{};
    std::uint64_t tributeRef{};
    std::uint64_t targetPoiRef{};
    std::uint64_t blueprintRef{};
    std::uint32_t weight{1};
};

struct OfferTableDefinition {
    std::uint64_t tableId{};
    std::uint32_t visibleOfferCount{3};
    std::vector<OfferDefinition> entries;
};

struct CourtOffer {
    std::uint64_t offerId{};
    std::uint64_t visitId{};
    std::uint64_t envoyId{};
    std::uint64_t playerId{};
    std::uint64_t systemId{};
    std::uint64_t definitionId{};
    CourtOfferKind kind{CourtOfferKind::Exchange};
    CourtOfferState state{CourtOfferState::Visible};
    std::uint64_t rewardRef{};
    std::uint64_t tributeRef{};
    std::uint64_t targetPoiRef{};
    std::uint64_t blueprintRef{};
    std::uint64_t createdTick{};
    std::uint64_t expiresTick{};
    std::uint64_t revision{1};
};

struct CourtVisit {
    std::uint64_t visitId{};
    std::uint64_t envoyId{};
    std::uint64_t playerId{};
    std::uint64_t systemId{};
    std::uint64_t createdTick{};
    std::uint64_t expiresTick{};
    std::uint64_t revision{1};
    std::uint64_t rollCycle{};
    CourtVisitState state{CourtVisitState::Visible};
    std::vector<std::uint64_t> visibleOfferIds;
};

struct CourtCommissionRecord {
    std::uint64_t commissionId{};
    std::uint64_t visitId{};
    std::uint64_t offerId{};
    std::uint64_t envoyId{};
    std::uint64_t playerId{};
    std::uint64_t systemId{};
    std::uint64_t targetPoiRef{};
    std::uint64_t blueprintRef{};
    std::uint64_t rewardRef{};
    std::uint64_t acceptedTick{};
    std::uint64_t revision{1};
};

struct CourtContext {
    std::uint64_t playerId{};
    std::uint64_t systemId{};
    double favor{};
    double suspicion{};
};

struct CourtSnapshot {
    std::uint64_t seed{};
    std::uint64_t considerationSerial{};
    std::vector<EnvoyDefinition> envoys;
    std::vector<OfferTableDefinition> tables;
    std::vector<CourtVisit> visits;
    std::vector<CourtOffer> offers;
    std::vector<CourtCommissionRecord> commissions;
};

class CourtOfferScheduler {
public:
    explicit CourtOfferScheduler(std::uint64_t seed = 0) : seed_(seed) {}

    bool publish(EnvoyDefinition definition);
    bool publish(OfferTableDefinition table);

    [[nodiscard]] std::vector<std::uint64_t> eligibleEnvoys(const CourtContext& context) const;
    [[nodiscard]] std::optional<CourtVisit> consider(const CourtContext& context, std::uint64_t tick);

    [[nodiscard]] const CourtVisit* visit(std::uint64_t visitId) const;
    [[nodiscard]] const CourtOffer* offer(std::uint64_t offerId) const;
    [[nodiscard]] const CourtCommissionRecord* commission(std::uint64_t commissionId) const;
    [[nodiscard]] std::vector<CourtOffer> visibleOffers(std::uint64_t visitId) const;

    // A failed transaction leaves the visible offer set untouched. A successful
    // transaction fulfills only the chosen offer and deterministically rerolls
    // the visit's next visible set afterward.
    bool commitTrade(std::uint64_t visitId,
                     std::uint64_t offerId,
                     std::uint64_t expectedVisitRevision,
                     bool transactionSucceeded,
                     std::uint64_t tick);

    bool transitionVisit(std::uint64_t visitId,
                         std::uint64_t expectedRevision,
                         CourtVisitState next,
                         std::uint64_t tick,
                         bool rerollAfterTransition = false);

    void expire(std::uint64_t tick);

    [[nodiscard]] CourtSnapshot snapshot() const;
    bool restore(const CourtSnapshot& snapshot);

    [[nodiscard]] static EnvoyDefinition canonicalEnvoy(EnvoyKind kind,
                                                        std::uint64_t offerTableId);
    [[nodiscard]] static const char* canonicalName(EnvoyKind kind) noexcept;

private:
    [[nodiscard]] bool eligible(const EnvoyDefinition& definition,
                                const CourtContext& context) const;
    [[nodiscard]] std::uint64_t deterministicId(std::uint64_t label,
                                                std::uint64_t a,
                                                std::uint64_t b,
                                                std::uint64_t c) const noexcept;
    bool reroll(CourtVisit& visit, std::uint64_t tick);
    bool materializeCommission(const CourtOffer& offer, std::uint64_t tick);

    std::uint64_t seed_{};
    std::uint64_t considerationSerial_{};
    std::map<std::uint64_t, EnvoyDefinition> envoys_;
    std::map<std::uint64_t, OfferTableDefinition> tables_;
    std::map<std::uint64_t, CourtVisit> visits_;
    std::map<std::uint64_t, CourtOffer> offers_;
    std::map<std::uint64_t, CourtCommissionRecord> commissions_;
};

} // namespace elysium::court

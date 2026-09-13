#include "court/CourtOfferScheduler.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace elysium::court {
namespace {
constexpr std::uint64_t kVisitLabel = 0x56495349545f434fULL;
constexpr std::uint64_t kOfferLabel = 0x4f464645525f434fULL;
constexpr std::uint64_t kCommissionLabel = 0x434f4d4d4953534eULL;

std::uint64_t mix64(std::uint64_t x) noexcept {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

constexpr std::uint64_t fnv1a(const char* text) noexcept {
    std::uint64_t h = 1469598103934665603ULL;
    while (*text) { h ^= static_cast<unsigned char>(*text++); h *= 1099511628211ULL; }
    return h;
}

bool terminal(CourtVisitState s) noexcept {
    return s == CourtVisitState::Completed || s == CourtVisitState::Expired || s == CourtVisitState::Cancelled;
}
}

const char* CourtOfferScheduler::canonicalName(EnvoyKind kind) noexcept {
    switch (kind) {
        case EnvoyKind::Elysomnion: return "Elysomnion";
        case EnvoyKind::SylpharaVoss: return "Sylphara Voss";
        case EnvoyKind::Sentinel: return "Sentinel";
        case EnvoyKind::Lillith: return "Lillith";
        case EnvoyKind::Aurelia: return "Aurelia";
    }
    return "Unknown";
}

EnvoyDefinition CourtOfferScheduler::canonicalEnvoy(EnvoyKind kind, std::uint64_t table) {
    EnvoyDefinition d{};
    d.kind = kind;
    d.offerTableId = table;
    switch (kind) {
        case EnvoyKind::Elysomnion:
            d.envoyId = fnv1a("elysium:court/envoy/elysomnion");
            d.officeId = fnv1a("elysium:court/office/emperor_black_emerald");
            d.meter = EligibilityMeter::SystemSuspicion; d.minimum = 75.0; break;
        case EnvoyKind::SylpharaVoss:
            d.envoyId = fnv1a("elysium:court/envoy/sylphara_voss");
            d.officeId = fnv1a("elysium:court/office/chief_imperial_architect");
            d.meter = EligibilityMeter::SystemSuspicion; d.minimum = 25.0; break;
        case EnvoyKind::Sentinel:
            d.envoyId = fnv1a("elysium:court/envoy/sentinel");
            d.officeId = fnv1a("elysium:court/office/stealth_envoy");
            d.meter = EligibilityMeter::Always; d.minimum = 0.0; break;
        case EnvoyKind::Lillith:
            d.envoyId = fnv1a("elysium:court/envoy/lillith");
            d.officeId = fnv1a("elysium:court/office/fleet_commander");
            d.meter = EligibilityMeter::Favor; d.minimum = 50.0; break;
        case EnvoyKind::Aurelia:
            d.envoyId = fnv1a("elysium:court/envoy/aurelia");
            d.officeId = fnv1a("elysium:court/office/queen");
            d.meter = EligibilityMeter::Favor; d.minimum = 75.0; break;
    }
    d.maximum = 100.0;
    return d;
}

bool CourtOfferScheduler::publish(EnvoyDefinition d) {
    if (!d.envoyId || !d.officeId || !d.offerTableId || !d.considerationIntervalTicks || !d.visitDurationTicks ||
        !std::isfinite(d.minimum) || !std::isfinite(d.maximum)) return false;
    d.minimum = safe::finiteClamp(d.minimum, 0.0, 100.0);
    d.maximum = safe::finiteClamp(d.maximum, d.minimum, 100.0);
    for (const auto& [_, existing] : envoys_) if (existing.kind == d.kind) return false;
    return envoys_.emplace(d.envoyId, d).second;
}

bool CourtOfferScheduler::publish(OfferTableDefinition table) {
    if (!table.tableId || table.entries.empty() || table.visibleOfferCount == 0) return false;
    std::set<std::uint64_t> ids;
    for (auto& e : table.entries) {
        if (!e.definitionId || !e.rewardRef || e.weight == 0 || !ids.insert(e.definitionId).second) return false;
        if (e.kind == CourtOfferKind::ConstructionCommission && (!e.targetPoiRef || !e.blueprintRef)) return false;
    }
    table.visibleOfferCount = std::min<std::uint32_t>(table.visibleOfferCount,
        safe::saturatingCast<std::uint32_t>(table.entries.size()));
    return tables_.emplace(table.tableId, std::move(table)).second;
}

bool CourtOfferScheduler::eligible(const EnvoyDefinition& d, const CourtContext& c) const {
    if (!c.playerId || !c.systemId || !tables_.contains(d.offerTableId)) return false;
    const auto& table = tables_.at(d.offerTableId);
    if (table.entries.empty()) return false;
    const double value = d.meter == EligibilityMeter::Favor ? c.favor :
        (d.meter == EligibilityMeter::SystemSuspicion ? c.suspicion : d.minimum);
    return std::isfinite(value) && safe::finiteClamp(value, 0.0, 100.0) >= d.minimum &&
           safe::finiteClamp(value, 0.0, 100.0) <= d.maximum;
}

std::vector<std::uint64_t> CourtOfferScheduler::eligibleEnvoys(const CourtContext& c) const {
    std::vector<std::uint64_t> out;
    for (const auto& [id, d] : envoys_) if (eligible(d, c)) out.push_back(id);
    return out;
}

std::uint64_t CourtOfferScheduler::deterministicId(std::uint64_t label,
                                                   std::uint64_t a,
                                                   std::uint64_t b,
                                                   std::uint64_t c) const noexcept {
    auto id = mix64(seed_ ^ label ^ mix64(a) ^ mix64(b) ^ mix64(c));
    return id ? id : 1;
}

bool CourtOfferScheduler::reroll(CourtVisit& visit, std::uint64_t tick) {
    const auto envoyIt = envoys_.find(visit.envoyId);
    if (envoyIt == envoys_.end()) return false;
    const auto tableIt = tables_.find(envoyIt->second.offerTableId);
    if (tableIt == tables_.end() || tableIt->second.entries.empty()) return false;

    for (auto id : visit.visibleOfferIds) {
        auto it = offers_.find(id);
        if (it != offers_.end() && it->second.state == CourtOfferState::Visible) {
            it->second.state = CourtOfferState::Withdrawn;
            it->second.revision = safe::saturatingIncrement(it->second.revision);
        }
    }
    visit.visibleOfferIds.clear();
    visit.rollCycle = safe::saturatingIncrement(visit.rollCycle);

    std::vector<std::size_t> pool(tableIt->second.entries.size());
    for (std::size_t i = 0; i < pool.size(); ++i) pool[i] = i;
    const auto requested = std::min<std::size_t>(tableIt->second.visibleOfferCount, pool.size());

    for (std::size_t slot = 0; slot < requested && !pool.empty(); ++slot) {
        std::uint64_t totalWeight = 0;
        for (auto index : pool) totalWeight = safe::saturatingAdd(totalWeight,
            static_cast<std::uint64_t>(tableIt->second.entries[index].weight));
        if (!totalWeight) return false;
        const auto roll = deterministicId(kOfferLabel, visit.visitId, visit.rollCycle, slot) % totalWeight;
        std::uint64_t cursor = 0;
        std::size_t selectedPoolIndex = 0;
        for (std::size_t p = 0; p < pool.size(); ++p) {
            cursor = safe::saturatingAdd(cursor,
                static_cast<std::uint64_t>(tableIt->second.entries[pool[p]].weight));
            if (roll < cursor) { selectedPoolIndex = p; break; }
        }
        const auto definition = tableIt->second.entries[pool[selectedPoolIndex]];
        pool.erase(pool.begin() + static_cast<std::ptrdiff_t>(selectedPoolIndex));

        CourtOffer offer{};
        offer.offerId = deterministicId(kOfferLabel, visit.visitId, visit.rollCycle,
                                         definition.definitionId ^ static_cast<std::uint64_t>(slot));
        while (offers_.contains(offer.offerId)) offer.offerId = mix64(offer.offerId + 1);
        offer.visitId = visit.visitId; offer.envoyId = visit.envoyId; offer.playerId = visit.playerId;
        offer.systemId = visit.systemId; offer.definitionId = definition.definitionId; offer.kind = definition.kind;
        offer.rewardRef = definition.rewardRef; offer.tributeRef = definition.tributeRef;
        offer.targetPoiRef = definition.targetPoiRef; offer.blueprintRef = definition.blueprintRef;
        offer.createdTick = tick; offer.expiresTick = visit.expiresTick;
        offers_.emplace(offer.offerId, offer);
        visit.visibleOfferIds.push_back(offer.offerId);
    }
    return !visit.visibleOfferIds.empty();
}

std::optional<CourtVisit> CourtOfferScheduler::consider(const CourtContext& c, std::uint64_t tick) {
    if (!c.playerId || !c.systemId || !std::isfinite(c.favor) || !std::isfinite(c.suspicion)) return std::nullopt;
    for (const auto& [_, visit] : visits_)
        if (!terminal(visit.state) && visit.playerId == c.playerId && visit.systemId == c.systemId) return std::nullopt;

    const auto candidates = eligibleEnvoys(c);
    if (candidates.empty()) return std::nullopt;
    considerationSerial_ = safe::saturatingIncrement(considerationSerial_);

    std::vector<std::uint64_t> due;
    for (auto id : candidates) {
        const auto& d = envoys_.at(id);
        if (tick % d.considerationIntervalTicks == 0) due.push_back(id);
    }
    if (due.empty()) return std::nullopt;

    const auto choice = deterministicId(kVisitLabel, c.playerId ^ c.systemId, tick, considerationSerial_);
    const auto envoyId = due[static_cast<std::size_t>(choice % due.size())];
    const auto& d = envoys_.at(envoyId);
    CourtVisit visit{};
    visit.visitId = deterministicId(kVisitLabel, envoyId, c.playerId ^ c.systemId, considerationSerial_);
    while (visits_.contains(visit.visitId)) visit.visitId = mix64(visit.visitId + 1);
    visit.envoyId = envoyId; visit.playerId = c.playerId; visit.systemId = c.systemId;
    visit.createdTick = tick; visit.expiresTick = safe::saturatingAdd(tick, d.visitDurationTicks);
    if (!reroll(visit, tick)) return std::nullopt;
    visits_.emplace(visit.visitId, visit);
    return visit;
}

const CourtVisit* CourtOfferScheduler::visit(std::uint64_t id) const { auto i=visits_.find(id); return i==visits_.end()?nullptr:&i->second; }
const CourtOffer* CourtOfferScheduler::offer(std::uint64_t id) const { auto i=offers_.find(id); return i==offers_.end()?nullptr:&i->second; }
const CourtCommissionRecord* CourtOfferScheduler::commission(std::uint64_t id) const { auto i=commissions_.find(id); return i==commissions_.end()?nullptr:&i->second; }

std::vector<CourtOffer> CourtOfferScheduler::visibleOffers(std::uint64_t visitId) const {
    std::vector<CourtOffer> out;
    const auto* v = visit(visitId); if (!v) return out;
    for (auto id : v->visibleOfferIds) { const auto* o=offer(id); if(o && o->state==CourtOfferState::Visible) out.push_back(*o); }
    return out;
}

bool CourtOfferScheduler::materializeCommission(const CourtOffer& offer, std::uint64_t tick) {
    if (offer.kind != CourtOfferKind::ConstructionCommission || !offer.targetPoiRef || !offer.blueprintRef) return true;
    CourtCommissionRecord c{};
    c.commissionId = deterministicId(kCommissionLabel, offer.visitId, offer.offerId, offer.definitionId);
    while (commissions_.contains(c.commissionId)) c.commissionId = mix64(c.commissionId + 1);
    c.visitId=offer.visitId; c.offerId=offer.offerId; c.envoyId=offer.envoyId; c.playerId=offer.playerId;
    c.systemId=offer.systemId; c.targetPoiRef=offer.targetPoiRef; c.blueprintRef=offer.blueprintRef;
    c.rewardRef=offer.rewardRef; c.acceptedTick=tick;
    return commissions_.emplace(c.commissionId,c).second;
}

bool CourtOfferScheduler::commitTrade(std::uint64_t visitId,
                                      std::uint64_t offerId,
                                      std::uint64_t expectedRevision,
                                      bool succeeded,
                                      std::uint64_t tick) {
    auto vit = visits_.find(visitId); if (vit==visits_.end()) return false;
    auto& v=vit->second;
    if(v.revision!=expectedRevision || v.state!=CourtVisitState::Visible || tick>v.expiresTick) return false;
    auto oit=offers_.find(offerId); if(oit==offers_.end() || oit->second.visitId!=visitId || oit->second.state!=CourtOfferState::Visible) return false;
    if(!succeeded) return true; // visible set stays exactly as shown; no tribute/standing mutation occurs here.
    auto& o=oit->second;
    if(!materializeCommission(o,tick)) return false;
    o.state=CourtOfferState::Fulfilled; o.revision=safe::saturatingIncrement(o.revision);
    v.revision=safe::saturatingIncrement(v.revision);
    // The caller may now apply tribute/standing effects using the committed offer receipt.
    return reroll(v,tick);
}

bool CourtOfferScheduler::transitionVisit(std::uint64_t id,std::uint64_t expected,CourtVisitState next,std::uint64_t tick,bool rerollSet){
    auto it=visits_.find(id); if(it==visits_.end()||it->second.revision!=expected||it->second.state==next||terminal(it->second.state))return false;
    auto&v=it->second; v.state=next; v.revision=safe::saturatingIncrement(v.revision);
    if(rerollSet && !terminal(next) && !reroll(v,tick))return false;
    return true;
}

void CourtOfferScheduler::expire(std::uint64_t tick){for(auto&[_,v]:visits_)if(!terminal(v.state)&&tick>=v.expiresTick){v.state=CourtVisitState::Expired;v.revision=safe::saturatingIncrement(v.revision);}}

CourtSnapshot CourtOfferScheduler::snapshot() const {
    CourtSnapshot s{};
    s.seed = seed_;
    s.considerationSerial = considerationSerial_;
    for (const auto& [_, v] : envoys_) s.envoys.push_back(v);
    for (const auto& [_, v] : tables_) s.tables.push_back(v);
    for (const auto& [_, v] : visits_) s.visits.push_back(v);
    for (const auto& [_, v] : offers_) s.offers.push_back(v);
    for (const auto& [_, v] : commissions_) s.commissions.push_back(v);
    return s;
}

bool CourtOfferScheduler::restore(const CourtSnapshot& s){
    CourtOfferScheduler next{s.seed};next.considerationSerial_=s.considerationSerial;
    for(auto d:s.envoys)if(!next.publish(d))return false;
    for(auto t:s.tables)if(!next.publish(t))return false;
    for(auto v:s.visits)if(!v.visitId||!v.envoyId||!v.playerId||!v.systemId||!v.revision||!next.envoys_.contains(v.envoyId)||!next.visits_.emplace(v.visitId,v).second)return false;
    for(auto o:s.offers)if(!o.offerId||!o.visitId||!o.revision||!next.visits_.contains(o.visitId)||!next.offers_.emplace(o.offerId,o).second)return false;
    for(auto c:s.commissions)if(!c.commissionId||!c.offerId||!c.revision||!next.offers_.contains(c.offerId)||!next.commissions_.emplace(c.commissionId,c).second)return false;
    for(auto&[_,v]:next.visits_)for(auto id:v.visibleOfferIds)if(!next.offers_.contains(id)||next.offers_.at(id).visitId!=v.visitId)return false;
    *this=std::move(next);return true;
}

} // namespace elysium::court

// Intended function: imported strategy implementation for StrategicSimulation; preserves the agent-authored subsystem contract for later integration/debugging.
#include "strategy/StrategicSimulation.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <limits>
#include <sstream>
#include <tuple>
#include <unordered_set>

namespace elysium {
namespace {

constexpr std::uint64_t kShipmentLabel = 0x5354524154454749ULL; // STRATEGI
constexpr std::uint64_t kEventLabel = 0x45564E5453545241ULL;    // EVNTSTRA
constexpr std::uint64_t kEfficiencyDenominator = 10000ULL * 10000ULL;
constexpr StrategicMinute kHourMinutes = 60;
constexpr StrategicMinute kDayMinutes = 24 * 60;
constexpr StrategicMinute kBlockedRetryMinutes = 60;

bool validBps(std::uint16_t value) {
    return value <= 10000;
}

template <typename T, typename Fn>
bool uniqueNonZeroBy(const std::vector<T>& values, Fn fn) {
    std::unordered_set<std::uint64_t> seen;
    for (const auto& value : values) {
        const auto id = static_cast<std::uint64_t>(fn(value));
        if (id == 0 || !seen.insert(id).second) return false;
    }
    return true;
}

bool validStockEntry(const StrategicStockEntry& entry) {
    return entry.itemId > 0 && entry.count > 0 && entry.unitMassGrams > 0 && entry.unitVolumeMl > 0;
}

bool validNamedItem(const StrategicNamedItem& item) {
    return item.stableId != 0 && item.itemId > 0 && item.massGrams > 0 && item.volumeMl > 0;
}

std::uint64_t saturatingProduct(std::uint64_t a, std::uint64_t b) {
    if (a == 0 || b == 0) return 0;
    if (a > std::numeric_limits<std::uint64_t>::max() / b) return std::numeric_limits<std::uint64_t>::max();
    return a * b;
}

} // namespace

const char* strategicRouteTypeName(StrategicRouteType type) {
    switch (type) {
        case StrategicRouteType::SurfaceOutpost: return "Surface Outpost";
        case StrategicRouteType::SurfaceOrbit: return "Surface-Orbit";
        case StrategicRouteType::PlanetPlanet: return "Planet-Planet";
        case StrategicRouteType::SystemSystem: return "System-System";
        case StrategicRouteType::RegionalNetwork: return "Regional Network";
    }
    return "Unknown";
}

const char* strategicShipmentStateName(StrategicShipmentState state) {
    switch (state) {
        case StrategicShipmentState::Loading: return "Loading";
        case StrategicShipmentState::InTransit: return "In Transit";
        case StrategicShipmentState::AwaitingUnload: return "Awaiting Unload";
        case StrategicShipmentState::Interrupted: return "Interrupted";
        case StrategicShipmentState::Delivered: return "Delivered";
        case StrategicShipmentState::Lost: return "Lost";
        case StrategicShipmentState::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

const char* strategicEventTypeName(StrategicEventType type) {
    switch (type) {
        case StrategicEventType::Raid: return "Raid";
        case StrategicEventType::Accident: return "Accident";
        case StrategicEventType::Inspection: return "Inspection";
        case StrategicEventType::RouteDelay: return "Route Delay";
        case StrategicEventType::ShipmentLost: return "Shipment Lost";
        case StrategicEventType::DeliveryBlocked: return "Delivery Blocked";
    }
    return "Unknown";
}

StrategicSimulation::StrategicSimulation(std::uint64_t campaignSeed)
    : campaignSeed_(campaignSeed) {}

void StrategicSimulation::normalizeStock(std::vector<StrategicStockEntry>& stock) {
    stock.erase(std::remove_if(stock.begin(), stock.end(), [](const auto& e) {
        return e.count <= 0;
    }), stock.end());
    std::sort(stock.begin(), stock.end(), [](const auto& a, const auto& b) {
        return std::tie(a.itemId, a.unitMassGrams, a.unitVolumeMl) <
               std::tie(b.itemId, b.unitMassGrams, b.unitVolumeMl);
    });
    std::vector<StrategicStockEntry> merged;
    for (const auto& entry : stock) {
        if (!merged.empty() && merged.back().itemId == entry.itemId &&
            merged.back().unitMassGrams == entry.unitMassGrams &&
            merged.back().unitVolumeMl == entry.unitVolumeMl) {
            merged.back().count += entry.count;
        } else {
            merged.push_back(entry);
        }
    }
    stock = std::move(merged);
}

std::uint64_t StrategicSimulation::stockMass(const std::vector<StrategicStockEntry>& stock,
                                             const std::vector<StrategicNamedItem>& namedItems) {
    std::uint64_t total = 0;
    for (const auto& entry : stock) {
        if (entry.count <= 0) continue;
        const auto add = saturatingProduct(static_cast<std::uint64_t>(entry.count), entry.unitMassGrams);
        if (add == std::numeric_limits<std::uint64_t>::max() || total > std::numeric_limits<std::uint64_t>::max() - add)
            return std::numeric_limits<std::uint64_t>::max();
        total += add;
    }
    for (const auto& item : namedItems) {
        if (total > std::numeric_limits<std::uint64_t>::max() - item.massGrams) return std::numeric_limits<std::uint64_t>::max();
        total += item.massGrams;
    }
    return total;
}

std::uint64_t StrategicSimulation::stockVolume(const std::vector<StrategicStockEntry>& stock,
                                               const std::vector<StrategicNamedItem>& namedItems) {
    std::uint64_t total = 0;
    for (const auto& entry : stock) {
        if (entry.count <= 0) continue;
        const auto add = saturatingProduct(static_cast<std::uint64_t>(entry.count), entry.unitVolumeMl);
        if (add == std::numeric_limits<std::uint64_t>::max() || total > std::numeric_limits<std::uint64_t>::max() - add)
            return std::numeric_limits<std::uint64_t>::max();
        total += add;
    }
    for (const auto& item : namedItems) {
        if (total > std::numeric_limits<std::uint64_t>::max() - item.volumeMl) return std::numeric_limits<std::uint64_t>::max();
        total += item.volumeMl;
    }
    return total;
}

std::int64_t StrategicSimulation::stockCount(const StrategicSiteState& site, int itemId) {
    std::int64_t result = 0;
    for (const auto& entry : site.stock) if (entry.itemId == itemId) result += entry.count;
    return result;
}

bool StrategicSimulation::removeStock(StrategicSiteState& site, int itemId, std::int64_t count) {
    if (count < 0 || stockCount(site, itemId) < count) return false;
    for (auto& entry : site.stock) {
        if (entry.itemId != itemId || count == 0) continue;
        const auto take = std::min(entry.count, count);
        entry.count -= take;
        count -= take;
    }
    normalizeStock(site.stock);
    return count == 0;
}

bool StrategicSimulation::insertStock(StrategicSiteState& site, StrategicStockEntry entry) {
    if (!validStockEntry(entry)) return false;
    for (const auto& existing : site.stock) {
        if (existing.itemId == entry.itemId &&
            (existing.unitMassGrams != entry.unitMassGrams || existing.unitVolumeMl != entry.unitVolumeMl)) return false;
    }
    std::vector<StrategicStockEntry> candidate = site.stock;
    candidate.push_back(entry);
    normalizeStock(candidate);
    if (stockMass(candidate, site.namedItems) > site.maxMassGrams ||
        stockVolume(candidate, site.namedItems) > site.maxVolumeMl) return false;
    site.stock = std::move(candidate);
    return true;
}

std::uint16_t StrategicSimulation::siteLaborAvailabilityBps(const StrategicSiteState& site) {
    std::uint64_t people = 0;
    std::uint64_t weighted = 0;
    for (const auto& cohort : site.cohorts) {
        people += cohort.population;
        weighted += static_cast<std::uint64_t>(cohort.population) * cohort.laborAvailabilityBps;
    }
    if (people == 0) return 10000; // autonomous/crewless remote installation
    return static_cast<std::uint16_t>(std::min<std::uint64_t>(10000, weighted / people));
}

bool StrategicSimulation::addSite(StrategicSiteState siteState, std::string* error) {
    auto fail = [&](const char* message) {
        if (error) *error = message;
        return false;
    };
    if (siteState.siteId == 0 || siteState.maxMassGrams == 0 || siteState.maxVolumeMl == 0 ||
        siteState.storedEnergyUnits > siteState.energyCapacityUnits || !validBps(siteState.infrastructureConditionBps))
        return fail("invalid strategic site identity/capacity/energy/condition");
    if (site(siteState.siteId)) return fail("duplicate strategic site ID");

    normalizeStock(siteState.stock);
    for (const auto& entry : siteState.stock) if (!validStockEntry(entry)) return fail("invalid strategic stock entry");
    for (std::size_t i = 1; i < siteState.stock.size(); ++i)
        if (siteState.stock[i-1].itemId == siteState.stock[i].itemId) return fail("stock item uses inconsistent mass/volume definitions");

    if (!uniqueNonZeroBy(siteState.namedItems, [](const auto& v){ return v.stableId; }) ||
        !uniqueNonZeroBy(siteState.namedFigures, [](const auto& v){ return v.stableId; }) ||
        !uniqueNonZeroBy(siteState.cohorts, [](const auto& v){ return v.stableId; }) ||
        !uniqueNonZeroBy(siteState.production, [](const auto& v){ return v.stableId; }) ||
        !uniqueNonZeroBy(siteState.consumption, [](const auto& v){ return v.stableId; }))
        return fail("duplicate/zero stable ID inside strategic site");

    for (const auto& item : siteState.namedItems) {
        if (!validNamedItem(item)) return fail("invalid named item");
        for (const auto& existingSite : sites_)
            for (const auto& existing : existingSite.namedItems)
                if (existing.stableId == item.stableId) return fail("named item stable ID already exists at another site");
        for (const auto& sh : shipments_)
            for (const auto& existing : sh.namedItems)
                if (existing.stableId == item.stableId) return fail("named item stable ID already exists in shipment");
    }
    for (const auto& figure : siteState.namedFigures) {
        for (const auto& existingSite : sites_)
            for (const auto& existing : existingSite.namedFigures)
                if (existing.stableId == figure.stableId) return fail("named figure stable ID already exists at another site");
    }
    for (const auto& cohort : siteState.cohorts) if (!validBps(cohort.laborAvailabilityBps)) return fail("invalid cohort labor availability");
    for (const auto& rule : siteState.production) {
        if (rule.outputItemId <= 0 || rule.outputCount <= 0 || rule.outputUnitMassGrams == 0 ||
            rule.outputUnitVolumeMl == 0 || rule.efficiencyRemainder >= kEfficiencyDenominator)
            return fail("invalid strategic production rule");
        for (const auto& input : rule.inputs) if (input.itemId <= 0 || input.count <= 0) return fail("invalid strategic production input");
    }
    for (const auto& rule : siteState.consumption)
        if (rule.itemId <= 0 || rule.fractionalRemainder >= 1000) return fail("invalid strategic consumption rule");

    if (stockMass(siteState.stock, siteState.namedItems) > siteState.maxMassGrams ||
        stockVolume(siteState.stock, siteState.namedItems) > siteState.maxVolumeMl)
        return fail("strategic site starts over stock capacity");

    auto byId = [](const auto& a, const auto& b){ return a.stableId < b.stableId; };
    std::sort(siteState.namedItems.begin(), siteState.namedItems.end(), byId);
    std::sort(siteState.namedFigures.begin(), siteState.namedFigures.end(), byId);
    std::sort(siteState.cohorts.begin(), siteState.cohorts.end(), byId);
    std::sort(siteState.production.begin(), siteState.production.end(), byId);
    std::sort(siteState.consumption.begin(), siteState.consumption.end(), byId);
    sites_.push_back(std::move(siteState));
    std::sort(sites_.begin(), sites_.end(), [](const auto& a, const auto& b){ return a.siteId < b.siteId; });
    return true;
}

bool StrategicSimulation::addRoute(StrategicRoute routeState, std::string* error) {
    auto fail = [&](const char* message) {
        if (error) *error = message;
        return false;
    };
    if (routeState.stableId == 0 || route(routeState.stableId) || routeState.fromSiteId == 0 || routeState.toSiteId == 0 ||
        routeState.fromSiteId == routeState.toSiteId || !site(routeState.fromSiteId) || !site(routeState.toSiteId) ||
        routeState.travelMinutes <= 0 || routeState.eventIntervalMinutes <= 0 || routeState.maxMassGrams == 0 ||
        routeState.maxVolumeMl == 0 || !validBps(routeState.hazardRiskBps) || !validBps(routeState.inspectionRiskBps) ||
        !validBps(routeState.lossRiskBps) || routeState.fuelPerDeparture < 0 ||
        (routeState.fuelPerDeparture > 0 && routeState.fuelItemId <= 0))
        return fail("invalid strategic route");
    routes_.push_back(routeState);
    std::sort(routes_.begin(), routes_.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    return true;
}

bool StrategicSimulation::addNamedItem(SiteId siteId, StrategicNamedItem item, std::string* error) {
    auto* s = site(siteId);
    if (!s || !validNamedItem(item)) {
        if (error) *error = "invalid site or named item";
        return false;
    }
    for (const auto& existingSite : sites_)
        for (const auto& existing : existingSite.namedItems)
            if (existing.stableId == item.stableId) {
                if (error) *error = "named item stable ID already exists";
                return false;
            }
    for (const auto& sh : shipments_)
        for (const auto& existing : sh.namedItems)
            if (existing.stableId == item.stableId) {
                if (error) *error = "named item stable ID already in shipment";
                return false;
            }
    auto candidate = s->namedItems;
    candidate.push_back(item);
    std::sort(candidate.begin(), candidate.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    if (stockMass(s->stock, candidate) > s->maxMassGrams || stockVolume(s->stock, candidate) > s->maxVolumeMl) {
        if (error) *error = "named item exceeds site capacity";
        return false;
    }
    s->namedItems = std::move(candidate);
    return true;
}

bool StrategicSimulation::addNamedFigure(SiteId siteId, StrategicNamedFigure figure, std::string* error) {
    auto* s = site(siteId);
    if (!s || figure.stableId == 0) {
        if (error) *error = "invalid site or named figure";
        return false;
    }
    for (const auto& existingSite : sites_)
        for (const auto& existing : existingSite.namedFigures)
            if (existing.stableId == figure.stableId) {
                if (error) *error = "named figure stable ID already exists";
                return false;
            }
    s->namedFigures.push_back(figure);
    std::sort(s->namedFigures.begin(), s->namedFigures.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    return true;
}

bool StrategicSimulation::addCohort(SiteId siteId, StrategicPopulationCohort cohort, std::string* error) {
    auto* s = site(siteId);
    if (!s || cohort.stableId == 0 || !validBps(cohort.laborAvailabilityBps)) {
        if (error) *error = "invalid site or population cohort";
        return false;
    }
    for (const auto& existing : s->cohorts) if (existing.stableId == cohort.stableId) {
        if (error) *error = "duplicate cohort stable ID";
        return false;
    }
    s->cohorts.push_back(cohort);
    std::sort(s->cohorts.begin(), s->cohorts.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    return true;
}

bool StrategicSimulation::addProductionRule(SiteId siteId, StrategicProductionRule rule, std::string* error) {
    auto* s = site(siteId);
    if (!s || rule.stableId == 0 || rule.outputItemId <= 0 || rule.outputCount <= 0 ||
        rule.outputUnitMassGrams == 0 || rule.outputUnitVolumeMl == 0 || rule.efficiencyRemainder >= kEfficiencyDenominator) {
        if (error) *error = "invalid production rule";
        return false;
    }
    for (const auto& input : rule.inputs) if (input.itemId <= 0 || input.count <= 0) {
        if (error) *error = "invalid production input";
        return false;
    }
    for (const auto& existing : s->production) if (existing.stableId == rule.stableId) {
        if (error) *error = "duplicate production rule stable ID";
        return false;
    }
    s->production.push_back(std::move(rule));
    std::sort(s->production.begin(), s->production.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    return true;
}

bool StrategicSimulation::addConsumptionRule(SiteId siteId, StrategicConsumptionRule rule, std::string* error) {
    auto* s = site(siteId);
    if (!s || rule.stableId == 0 || rule.itemId <= 0 || rule.fractionalRemainder >= 1000) {
        if (error) *error = "invalid consumption rule";
        return false;
    }
    for (const auto& existing : s->consumption) if (existing.stableId == rule.stableId) {
        if (error) *error = "duplicate consumption rule stable ID";
        return false;
    }
    s->consumption.push_back(rule);
    std::sort(s->consumption.begin(), s->consumption.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    return true;
}

bool StrategicSimulation::addStock(SiteId siteId, StrategicStockEntry entry, std::string* error) {
    auto* s = site(siteId);
    if (!s || !insertStock(*s, entry)) {
        if (error) *error = "stock insert invalid or exceeds capacity";
        return false;
    }
    return true;
}

const StrategicSiteState* StrategicSimulation::site(SiteId siteId) const {
    const auto it = std::lower_bound(sites_.begin(), sites_.end(), siteId, [](const auto& s, SiteId id){ return s.siteId < id; });
    return it != sites_.end() && it->siteId == siteId ? &*it : nullptr;
}

StrategicSiteState* StrategicSimulation::site(SiteId siteId) {
    const auto it = std::lower_bound(sites_.begin(), sites_.end(), siteId, [](const auto& s, SiteId id){ return s.siteId < id; });
    return it != sites_.end() && it->siteId == siteId ? &*it : nullptr;
}

const StrategicRoute* StrategicSimulation::route(std::uint64_t stableId) const {
    const auto it = std::lower_bound(routes_.begin(), routes_.end(), stableId, [](const auto& r, std::uint64_t id){ return r.stableId < id; });
    return it != routes_.end() && it->stableId == stableId ? &*it : nullptr;
}

const StrategicShipment* StrategicSimulation::shipment(std::uint64_t stableId) const {
    const auto it = std::lower_bound(shipments_.begin(), shipments_.end(), stableId, [](const auto& s, std::uint64_t id){ return s.stableId < id; });
    return it != shipments_.end() && it->stableId == stableId ? &*it : nullptr;
}

std::uint64_t StrategicSimulation::allocateShipmentId(std::uint64_t routeStableId) {
    for (;;) {
        const auto id = mix64(campaignSeed_ ^ kShipmentLabel ^ routeStableId ^ nextShipmentSerial_++);
        if (id != 0 && !shipment(id)) return id;
    }
}

std::uint64_t StrategicSimulation::allocateEventId(std::uint64_t shipmentStableId,
                                                   std::uint32_t ordinal,
                                                   StrategicEventType type) {
    for (;;) {
        const auto serial = nextEventSerial_++;
        const auto id = mix64(campaignSeed_ ^ kEventLabel ^ shipmentStableId ^
                              (static_cast<std::uint64_t>(ordinal) << 24U) ^
                              (static_cast<std::uint64_t>(type) << 56U) ^ serial);
        if (id != 0) return id;
    }
}

std::optional<std::uint64_t> StrategicSimulation::dispatch(std::uint64_t routeStableId,
                                                           std::uint64_t vehicleStableId,
                                                           std::vector<StrategicStockEntry> manifest,
                                                           const std::vector<std::uint64_t>& namedItemIds,
                                                           std::string* error) {
    auto fail = [&](const char* message) -> std::optional<std::uint64_t> {
        if (error) *error = message;
        return std::nullopt;
    };
    const auto* r = route(routeStableId);
    if (!r || !r->enabled) return fail("route missing or disabled");
    auto* origin = site(r->fromSiteId);
    auto* destination = site(r->toSiteId);
    if (!origin || !destination) return fail("route endpoint missing");
    if (origin->mode != StrategicSiteMode::Strategic || destination->mode != StrategicSiteMode::Strategic)
        return fail("strategic dispatch requires remote strategic endpoints");

    normalizeStock(manifest);
    for (const auto& entry : manifest) if (!validStockEntry(entry)) return fail("invalid shipment manifest entry");
    std::unordered_set<std::uint64_t> requestedNamed;
    std::vector<StrategicNamedItem> namedCargo;
    for (const auto id : namedItemIds) {
        if (id == 0 || !requestedNamed.insert(id).second) return fail("duplicate/zero named item in shipment request");
        const auto it = std::find_if(origin->namedItems.begin(), origin->namedItems.end(), [&](const auto& item){ return item.stableId == id; });
        if (it == origin->namedItems.end()) return fail("named shipment item not owned by origin");
        namedCargo.push_back(*it);
    }

    for (const auto& entry : manifest) {
        const auto it = std::find_if(origin->stock.begin(), origin->stock.end(), [&](const auto& stock) { return stock.itemId == entry.itemId; });
        if (it == origin->stock.end() || it->count < entry.count) return fail("shipment manifest exceeds origin stock");
        if (it->unitMassGrams != entry.unitMassGrams || it->unitVolumeMl != entry.unitVolumeMl)
            return fail("shipment manifest item physical properties disagree with origin stock");
    }
    if (stockMass(manifest, namedCargo) > r->maxMassGrams || stockVolume(manifest, namedCargo) > r->maxVolumeMl)
        return fail("shipment exceeds route mass/volume capacity");

    StrategicSiteState staged = *origin;
    for (const auto& entry : manifest) if (!removeStock(staged, entry.itemId, entry.count)) return fail("shipment stock reservation failed");
    if (r->fuelPerDeparture > 0 && !removeStock(staged, r->fuelItemId, r->fuelPerDeparture)) return fail("insufficient route fuel");
    if (r->energyPerDeparture > staged.storedEnergyUnits) return fail("insufficient strategic route energy reserve");
    staged.storedEnergyUnits -= r->energyPerDeparture;
    staged.namedItems.erase(std::remove_if(staged.namedItems.begin(), staged.namedItems.end(), [&](const auto& item) {
        return requestedNamed.contains(item.stableId);
    }), staged.namedItems.end());
    *origin = std::move(staged);

    StrategicShipment sh{};
    sh.stableId = allocateShipmentId(routeStableId);
    sh.routeStableId = routeStableId;
    sh.vehicleStableId = vehicleStableId;
    sh.state = StrategicShipmentState::InTransit;
    sh.departedMinute = currentMinute_;
    sh.expectedArrivalMinute = currentMinute_ + r->travelMinutes;
    sh.nextEventMinute = currentMinute_ + r->eventIntervalMinutes;
    sh.manifest = std::move(manifest);
    sh.namedItems = std::move(namedCargo);
    const auto newShipmentId = sh.stableId;
    shipments_.push_back(std::move(sh));
    std::sort(shipments_.begin(), shipments_.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    ++telemetry_.shipmentsDispatched;
    return newShipmentId;
}

bool StrategicSimulation::processHour(StrategicMinute hourMinute) {
    ++telemetry_.hourlySiteTicks;
    const bool dayBoundary = hourMinute % kDayMinutes == 0;
    if (dayBoundary) ++telemetry_.dailyConsumptionTicks;
    for (auto& s : sites_) {
        if (s.mode != StrategicSiteMode::Strategic) continue;
        s.shortages.clear();
        if (dayBoundary) processSiteConsumption(s);
        processSiteProduction(s);
    }
    return true;
}

bool StrategicSimulation::processDay(StrategicMinute) {
    return true;
}

void StrategicSimulation::processSiteProduction(StrategicSiteState& s) {
    const auto labor = siteLaborAvailabilityBps(s);
    for (auto& rule : s.production) {
        const std::uint64_t numerator = static_cast<std::uint64_t>(rule.batchesPerHour) * labor * s.infrastructureConditionBps + rule.efficiencyRemainder;
        const auto desired = numerator / kEfficiencyDenominator;
        rule.efficiencyRemainder = numerator % kEfficiencyDenominator;
        if (desired == 0) continue;

        std::uint64_t actual = desired;
        int limitingItem = 0;
        std::int64_t limitingRequired = 0;
        std::int64_t limitingAvailable = 0;
        for (const auto& input : rule.inputs) {
            const auto available = stockCount(s, input.itemId);
            const auto possible = static_cast<std::uint64_t>(available / input.count);
            if (possible < actual) {
                actual = possible;
                limitingItem = input.itemId;
                limitingRequired = static_cast<std::int64_t>(desired) * input.count;
                limitingAvailable = available;
            }
        }

        const auto currentMass = stockMass(s.stock, s.namedItems);
        const auto currentVolume = stockVolume(s.stock, s.namedItems);
        const auto perBatchMass = saturatingProduct(static_cast<std::uint64_t>(rule.outputCount), rule.outputUnitMassGrams);
        const auto perBatchVolume = saturatingProduct(static_cast<std::uint64_t>(rule.outputCount), rule.outputUnitVolumeMl);
        if (perBatchMass > 0) actual = std::min<std::uint64_t>(actual, (s.maxMassGrams - std::min(s.maxMassGrams, currentMass)) / perBatchMass);
        if (perBatchVolume > 0) actual = std::min<std::uint64_t>(actual, (s.maxVolumeMl - std::min(s.maxVolumeMl, currentVolume)) / perBatchVolume);

        if (actual == 0) {
            if (limitingItem != 0) s.shortages.push_back({limitingItem, limitingRequired, limitingAvailable, "remote production lacks input stock"});
            else s.shortages.push_back({rule.outputItemId, static_cast<std::int64_t>(desired * rule.outputCount), 0, "remote production output capacity is full"});
            continue;
        }

        for (const auto& input : rule.inputs) removeStock(s, input.itemId, static_cast<std::int64_t>(actual) * input.count);
        const StrategicStockEntry out{rule.outputItemId, static_cast<std::int64_t>(actual) * rule.outputCount,
                                      rule.outputUnitMassGrams, rule.outputUnitVolumeMl};
        if (!insertStock(s, out)) {
            // Capacity was precomputed; reaching this branch indicates an invariant failure.
            s.shortages.push_back({rule.outputItemId, out.count, 0, "remote production insert invariant failed"});
            continue;
        }
        telemetry_.productionBatches += actual;
        if (actual < desired && limitingItem != 0)
            s.shortages.push_back({limitingItem, limitingRequired, limitingAvailable, "remote production input limited throughput"});
    }
}

void StrategicSimulation::processSiteConsumption(StrategicSiteState& s) {
    std::uint64_t population = 0;
    for (const auto& cohort : s.cohorts) population += cohort.population;
    for (const auto& figure : s.namedFigures) if (figure.alive) ++population;
    if (population == 0) return;

    for (auto& rule : s.consumption) {
        const std::uint64_t numerator = population * static_cast<std::uint64_t>(rule.milliUnitsPerPersonPerDay) + rule.fractionalRemainder;
        const auto requiredUnsigned = numerator / 1000U;
        const auto required = requiredUnsigned > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())
            ? std::numeric_limits<std::int64_t>::max()
            : static_cast<std::int64_t>(requiredUnsigned);
        rule.fractionalRemainder = numerator % 1000U;
        if (required <= 0) continue;
        const auto available = stockCount(s, rule.itemId);
        const auto consumed = std::min(required, available);
        if (consumed > 0) removeStock(s, rule.itemId, consumed);
        if (consumed < required) s.shortages.push_back({rule.itemId, required, available, "remote population consumption shortage"});
    }
}

void StrategicSimulation::recordEvent(StrategicEventRecord event) {
    if (event.stableId == 0) event.stableId = allocateEventId(event.shipmentStableId, 0, event.type);
    events_.push_back(event);
    ++telemetry_.strategicEvents;
}

void StrategicSimulation::processShipmentEvent(StrategicShipment& sh, const StrategicRoute& r, StrategicMinute minute) {
    const std::uint32_t ordinal = sh.eventOrdinal++;
    const auto base = mix64(campaignSeed_ ^ sh.stableId ^ r.stableId ^
                            (static_cast<std::uint64_t>(ordinal + 1U) * 0x9E3779B97F4A7C15ULL));
    const auto hazardRoll = static_cast<std::uint16_t>(base % 10000ULL);
    const auto inspectRoll = static_cast<std::uint16_t>(mix64(base ^ 0x494E5350454354ULL) % 10000ULL);
    const auto lossRoll = static_cast<std::uint16_t>(mix64(base ^ 0x4C4F53535249534BULL) % 10000ULL);

    if (lossRoll < r.lossRiskBps) {
        sh.state = StrategicShipmentState::Lost;
        ++telemetry_.shipmentsLost;
        recordEvent({allocateEventId(sh.stableId, ordinal, StrategicEventType::ShipmentLost), StrategicEventType::ShipmentLost,
                     minute, sh.stableId, r.stableId, 0, 0, 0, 0});
        return;
    }

    StrategicMinute interruptionDelay = 0;
    if (inspectRoll < r.inspectionRiskBps) {
        constexpr StrategicMinute delay = 30;
        interruptionDelay += delay;
        sh.delayMinutes += delay;
        sh.expectedArrivalMinute += delay;
        recordEvent({allocateEventId(sh.stableId, ordinal, StrategicEventType::Inspection), StrategicEventType::Inspection,
                     minute, sh.stableId, r.stableId, r.toSiteId, 0, 0, delay});
    }

    if (hazardRoll < r.hazardRiskBps) {
        const auto type = ((base >> 48U) & 1U) ? StrategicEventType::Raid : StrategicEventType::Accident;
        StrategicMinute delay = type == StrategicEventType::Raid ? 120 : 60;
        int lostItem = 0;
        std::int64_t lostQuantity = 0;
        if (!sh.manifest.empty()) {
            const auto index = static_cast<std::size_t>((base >> 40U) % sh.manifest.size());
            auto& entry = sh.manifest[index];
            if (entry.count > 0) {
                lostQuantity = std::max<std::int64_t>(1, entry.count / 4);
                lostItem = entry.itemId;
                entry.count -= lostQuantity;
                normalizeStock(sh.manifest);
            }
        }
        interruptionDelay += delay;
        sh.delayMinutes += delay;
        sh.expectedArrivalMinute += delay;
        recordEvent({allocateEventId(sh.stableId, ordinal, type), type, minute, sh.stableId, r.stableId,
                     0, lostItem, lostQuantity, delay});
    }

    // Delays are real strategic interruptions rather than merely moving the ETA.
    // This matters for promotion: detailed simulation must know the convoy is stopped,
    // and no second remote hazard/inspection may occur while that stop is active.
    if (interruptionDelay > 0) {
        sh.state = StrategicShipmentState::Interrupted;
        sh.interruptionUntilMinute = minute + interruptionDelay;
        sh.nextEventMinute = sh.interruptionUntilMinute + r.eventIntervalMinutes;
    } else {
        sh.nextEventMinute = minute + r.eventIntervalMinutes;
    }
}

void StrategicSimulation::tryUnload(StrategicShipment& sh, const StrategicRoute& r, StrategicMinute minute) {
    auto* destination = site(r.toSiteId);
    if (!destination || destination->mode != StrategicSiteMode::Strategic) {
        sh.state = StrategicShipmentState::AwaitingUnload;
        sh.nextEventMinute = minute + kBlockedRetryMinutes;
        ++telemetry_.blockedDeliveries;
        recordEvent({0, StrategicEventType::DeliveryBlocked, minute, sh.stableId, r.stableId, r.toSiteId, 0, 0, 0});
        return;
    }

    auto candidate = *destination;
    bool canFit = true;
    for (const auto& entry : sh.manifest) if (!insertStock(candidate, entry)) { canFit = false; break; }
    if (canFit) {
        for (const auto& item : sh.namedItems) {
            candidate.namedItems.push_back(item);
            std::sort(candidate.namedItems.begin(), candidate.namedItems.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
            if (stockMass(candidate.stock, candidate.namedItems) > candidate.maxMassGrams ||
                stockVolume(candidate.stock, candidate.namedItems) > candidate.maxVolumeMl) { canFit = false; break; }
        }
    }

    if (!canFit) {
        sh.state = StrategicShipmentState::AwaitingUnload;
        sh.nextEventMinute = minute + kBlockedRetryMinutes;
        ++telemetry_.blockedDeliveries;
        recordEvent({0, StrategicEventType::DeliveryBlocked, minute, sh.stableId, r.stableId, r.toSiteId, 0, 0, 0});
        return;
    }

    *destination = std::move(candidate);
    sh.manifest.clear();
    sh.namedItems.clear();
    sh.state = StrategicShipmentState::Delivered;
    ++telemetry_.shipmentsDelivered;
}

void StrategicSimulation::processShipmentsAt(StrategicMinute minute) {
    for (auto& sh : shipments_) {
        if (sh.promotedDetailed || sh.state == StrategicShipmentState::Delivered || sh.state == StrategicShipmentState::Lost ||
            sh.state == StrategicShipmentState::Cancelled) continue;
        const auto* r = route(sh.routeStableId);
        if (!r) continue;
        if (sh.state == StrategicShipmentState::AwaitingUnload) {
            if (minute >= sh.nextEventMinute) tryUnload(sh, *r, minute);
            continue;
        }
        if (sh.state == StrategicShipmentState::Interrupted) {
            if (minute < sh.interruptionUntilMinute) continue;
            sh.state = StrategicShipmentState::InTransit;
            sh.interruptionUntilMinute = 0;
        }
        if (minute >= sh.expectedArrivalMinute) {
            tryUnload(sh, *r, minute);
            continue;
        }
        if (minute >= sh.nextEventMinute) processShipmentEvent(sh, *r, minute);
    }
}

StrategicMinute StrategicSimulation::nextBoundaryAfter(StrategicMinute minute, StrategicMinute target) const {
    StrategicMinute next = target;
    const auto hour = ((minute / kHourMinutes) + 1) * kHourMinutes;
    if (hour > minute && hour < next) next = hour;
    for (const auto& sh : shipments_) {
        if (sh.promotedDetailed || sh.state == StrategicShipmentState::Delivered || sh.state == StrategicShipmentState::Lost ||
            sh.state == StrategicShipmentState::Cancelled) continue;
        if (sh.state == StrategicShipmentState::AwaitingUnload) {
            if (sh.nextEventMinute > minute && sh.nextEventMinute < next) next = sh.nextEventMinute;
            continue;
        }
        if (sh.state == StrategicShipmentState::Interrupted &&
            sh.interruptionUntilMinute > minute && sh.interruptionUntilMinute < next)
            next = sh.interruptionUntilMinute;
        if (sh.expectedArrivalMinute > minute && sh.expectedArrivalMinute < next) next = sh.expectedArrivalMinute;
        if (sh.nextEventMinute > minute && sh.nextEventMinute < next && sh.nextEventMinute < sh.expectedArrivalMinute)
            next = sh.nextEventMinute;
    }
    return next;
}

bool StrategicSimulation::advanceTo(StrategicMinute targetMinute, std::string* error) {
    if (targetMinute < currentMinute_) {
        if (error) *error = "strategic clock cannot run backward";
        return false;
    }
    while (currentMinute_ < targetMinute) {
        const auto next = nextBoundaryAfter(currentMinute_, targetMinute);
        if (next <= currentMinute_) {
            if (error) *error = "strategic boundary scheduler failed to advance";
            return false;
        }
        currentMinute_ = next;
        if (currentMinute_ > 0 && currentMinute_ % kHourMinutes == 0) processHour(currentMinute_);
        processShipmentsAt(currentMinute_);
    }
    return true;
}

std::optional<StrategicSitePromotion> StrategicSimulation::promoteSite(SiteId siteId, std::string* error) {
    auto* s = site(siteId);
    if (!s || s->mode != StrategicSiteMode::Strategic) {
        if (error) *error = "site missing or already detailed";
        return std::nullopt;
    }
    s->mode = StrategicSiteMode::Detailed;
    return StrategicSitePromotion{siteId, currentMinute_, s->stock, s->namedItems, s->namedFigures, s->cohorts};
}

bool StrategicSimulation::compactSite(const StrategicSitePromotion& snapshot, std::string* error) {
    auto* s = site(snapshot.siteId);
    if (!s || s->mode != StrategicSiteMode::Detailed) {
        if (error) *error = "site missing or not in detailed mode";
        return false;
    }
    StrategicSiteState candidate = *s;
    candidate.stock = snapshot.stock;
    normalizeStock(candidate.stock);
    candidate.namedItems = snapshot.namedItems;
    candidate.namedFigures = snapshot.namedFigures;
    candidate.cohorts = snapshot.cohorts;
    candidate.mode = StrategicSiteMode::Strategic;
    if (!uniqueNonZeroBy(candidate.namedItems, [](const auto& v){ return v.stableId; }) ||
        !uniqueNonZeroBy(candidate.namedFigures, [](const auto& v){ return v.stableId; }) ||
        !uniqueNonZeroBy(candidate.cohorts, [](const auto& v){ return v.stableId; }) ||
        stockMass(candidate.stock, candidate.namedItems) > candidate.maxMassGrams ||
        stockVolume(candidate.stock, candidate.namedItems) > candidate.maxVolumeMl) {
        if (error) *error = "detailed site snapshot violates strategic identity/capacity invariants";
        return false;
    }
    for (const auto& item : candidate.namedItems) {
        for (const auto& otherSite : sites_) {
            if (otherSite.siteId == candidate.siteId) continue;
            for (const auto& existing : otherSite.namedItems) if (existing.stableId == item.stableId) {
                if (error) *error = "detailed site snapshot duplicates named item at another site";
                return false;
            }
        }
        for (const auto& sh : shipments_)
            for (const auto& existing : sh.namedItems) if (existing.stableId == item.stableId) {
                if (error) *error = "detailed site snapshot duplicates named item in shipment";
                return false;
            }
    }
    for (const auto& figure : candidate.namedFigures)
        for (const auto& otherSite : sites_) {
            if (otherSite.siteId == candidate.siteId) continue;
            for (const auto& existing : otherSite.namedFigures) if (existing.stableId == figure.stableId) {
                if (error) *error = "detailed site snapshot duplicates named figure at another site";
                return false;
            }
        }
    *s = std::move(candidate);
    return true;
}

std::optional<StrategicShipmentPromotion> StrategicSimulation::promoteShipment(std::uint64_t shipmentStableId, std::string* error) {
    auto it = std::lower_bound(shipments_.begin(), shipments_.end(), shipmentStableId,
                               [](const auto& sh, std::uint64_t id){ return sh.stableId < id; });
    if (it == shipments_.end() || it->stableId != shipmentStableId || it->promotedDetailed ||
        it->state == StrategicShipmentState::Delivered || it->state == StrategicShipmentState::Lost ||
        it->state == StrategicShipmentState::Cancelled) {
        if (error) *error = "shipment missing or cannot be promoted";
        return std::nullopt;
    }
    it->promotedDetailed = true;
    StrategicShipmentPromotion out{};
    out.shipmentStableId = shipmentStableId;
    out.strategicMinute = currentMinute_;
    out.state = it->state;
    out.remainingMinutesUntilArrival = std::max<StrategicMinute>(0, it->expectedArrivalMinute - currentMinute_);
    out.remainingMinutesUntilEvent = std::max<StrategicMinute>(0, it->nextEventMinute - currentMinute_);
    out.remainingInterruptionMinutes = it->state == StrategicShipmentState::Interrupted
        ? std::max<StrategicMinute>(0, it->interruptionUntilMinute - currentMinute_) : 0;
    out.manifest = it->manifest;
    out.namedItems = it->namedItems;
    return out;
}

bool StrategicSimulation::compactShipment(const StrategicShipmentPromotion& snapshot, std::string* error) {
    auto it = std::lower_bound(shipments_.begin(), shipments_.end(), snapshot.shipmentStableId,
                               [](const auto& sh, std::uint64_t id){ return sh.stableId < id; });
    if (it == shipments_.end() || it->stableId != snapshot.shipmentStableId || !it->promotedDetailed) {
        if (error) *error = "shipment missing or not promoted";
        return false;
    }
    auto manifest = snapshot.manifest;
    normalizeStock(manifest);
    for (const auto& entry : manifest) if (!validStockEntry(entry)) {
        if (error) *error = "invalid compacted shipment manifest";
        return false;
    }
    if (!uniqueNonZeroBy(snapshot.namedItems, [](const auto& v){ return v.stableId; })) {
        if (error) *error = "invalid compacted shipment named items";
        return false;
    }
    for (const auto& item : snapshot.namedItems) {
        for (const auto& existingSite : sites_)
            for (const auto& existing : existingSite.namedItems) if (existing.stableId == item.stableId) {
                if (error) *error = "compacted shipment duplicates named item at a site";
                return false;
            }
        for (const auto& otherShipment : shipments_) {
            if (otherShipment.stableId == snapshot.shipmentStableId) continue;
            for (const auto& existing : otherShipment.namedItems) if (existing.stableId == item.stableId) {
                if (error) *error = "compacted shipment duplicates named item in another shipment";
                return false;
            }
        }
    }
    it->state = snapshot.state;
    it->manifest = std::move(manifest);
    it->namedItems = snapshot.namedItems;
    it->expectedArrivalMinute = currentMinute_ + std::max<StrategicMinute>(0, snapshot.remainingMinutesUntilArrival);
    it->nextEventMinute = currentMinute_ + std::max<StrategicMinute>(0, snapshot.remainingMinutesUntilEvent);
    it->interruptionUntilMinute = snapshot.state == StrategicShipmentState::Interrupted
        ? currentMinute_ + std::max<StrategicMinute>(0, snapshot.remainingInterruptionMinutes) : 0;
    it->promotedDetailed = false;
    return true;
}

std::vector<StrategicDependencyEdge> StrategicSimulation::dependencyGraph(SiteId siteId) const {
    std::vector<StrategicDependencyEdge> out;
    for (const auto& r : routes_) {
        if (r.fromSiteId != siteId && r.toSiteId != siteId) continue;
        std::uint32_t active = 0;
        for (const auto& sh : shipments_) if (sh.routeStableId == r.stableId &&
            sh.state != StrategicShipmentState::Delivered && sh.state != StrategicShipmentState::Lost &&
            sh.state != StrategicShipmentState::Cancelled) ++active;
        out.push_back({r.stableId, r.fromSiteId, r.toSiteId, active});
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.routeStableId < b.routeStableId; });
    return out;
}

std::string StrategicSimulation::explainSite(SiteId siteId) const {
    const auto* s = site(siteId);
    if (!s) return "site not found";
    std::ostringstream out;
    out << "Site " << s->siteId << " is " << (s->mode == StrategicSiteMode::Strategic ? "strategic" : "detailed")
        << "; stock mass " << stockMass(s->stock, s->namedItems) << '/' << s->maxMassGrams
        << " g, volume " << stockVolume(s->stock, s->namedItems) << '/' << s->maxVolumeMl
        << " ml; energy " << s->storedEnergyUnits << '/' << s->energyCapacityUnits
        << "; labor availability " << siteLaborAvailabilityBps(*s) << " bps; infrastructure "
        << s->infrastructureConditionBps << " bps.";
    if (!s->shortages.empty()) {
        out << " Shortages:";
        for (const auto& shortage : s->shortages)
            out << " item " << shortage.itemId << " needs " << shortage.required << ", has " << shortage.available
                << " (" << shortage.reason << ");";
    }
    const auto deps = dependencyGraph(siteId);
    out << " Routes " << deps.size() << '.';
    return out.str();
}

std::string StrategicSimulation::explainShipment(std::uint64_t shipmentStableId) const {
    const auto* sh = shipment(shipmentStableId);
    if (!sh) return "shipment not found";
    const auto* r = route(sh->routeStableId);
    std::ostringstream out;
    out << "Shipment " << sh->stableId << " is " << strategicShipmentStateName(sh->state)
        << "; route " << sh->routeStableId;
    if (r) out << " (" << strategicRouteTypeName(r->type) << ", " << r->fromSiteId << " -> " << r->toSiteId << ')';
    out << "; ETA minute " << sh->expectedArrivalMinute
        << "; accumulated delay " << sh->delayMinutes << " minutes";
    if (sh->state == StrategicShipmentState::Interrupted)
        out << "; interrupted until minute " << sh->interruptionUntilMinute;
    out << "; aggregate lines " << sh->manifest.size() << "; named cargo " << sh->namedItems.size() << '.';
    return out.str();
}

std::string StrategicSimulation::serializeState() const {
    std::ostringstream out;
    out << "ELYSIUM_STRATEGIC 2\n";
    out << "META " << campaignSeed_ << ' ' << currentMinute_ << ' ' << nextShipmentSerial_ << ' ' << nextEventSerial_ << '\n';
    out << "SITES " << sites_.size() << '\n';
    for (const auto& s : sites_) {
        out << "SITE " << s.siteId << ' ' << s.systemId << ' ' << s.planetId << ' ' << static_cast<int>(s.mode) << ' '
            << s.maxMassGrams << ' ' << s.maxVolumeMl << ' ' << s.storedEnergyUnits << ' ' << s.energyCapacityUnits << ' '
            << s.infrastructureConditionBps << ' '
            << s.stock.size() << ' ' << s.namedItems.size() << ' ' << s.namedFigures.size() << ' ' << s.cohorts.size() << ' '
            << s.production.size() << ' ' << s.consumption.size() << '\n';
        for (const auto& e : s.stock) out << "ST " << e.itemId << ' ' << e.count << ' ' << e.unitMassGrams << ' ' << e.unitVolumeMl << '\n';
        for (const auto& e : s.namedItems) out << "NI " << e.stableId << ' ' << e.itemId << ' ' << e.massGrams << ' ' << e.volumeMl << '\n';
        for (const auto& e : s.namedFigures) out << "NF " << e.stableId << ' ' << e.roleId << ' ' << (e.alive ? 1 : 0) << '\n';
        for (const auto& e : s.cohorts) out << "CO " << e.stableId << ' ' << e.population << ' ' << e.laborAvailabilityBps << ' ' << e.cohortTypeId << '\n';
        for (const auto& e : s.production) {
            out << "PR " << e.stableId << ' ' << e.outputItemId << ' ' << e.outputCount << ' ' << e.outputUnitMassGrams << ' '
                << e.outputUnitVolumeMl << ' ' << e.batchesPerHour << ' ' << e.efficiencyRemainder << ' ' << e.inputs.size();
            for (const auto& input : e.inputs) out << ' ' << input.itemId << ' ' << input.count;
            out << '\n';
        }
        for (const auto& e : s.consumption) out << "CR " << e.stableId << ' ' << e.itemId << ' ' << e.milliUnitsPerPersonPerDay << ' ' << e.fractionalRemainder << '\n';
    }
    out << "ROUTES " << routes_.size() << '\n';
    for (const auto& r : routes_) out << "RT " << r.stableId << ' ' << static_cast<int>(r.type) << ' ' << r.fromSiteId << ' ' << r.toSiteId << ' '
        << r.travelMinutes << ' ' << r.maxMassGrams << ' ' << r.maxVolumeMl << ' ' << r.fuelItemId << ' ' << r.fuelPerDeparture << ' '
        << r.energyPerDeparture << ' ' << r.hazardRiskBps << ' ' << r.inspectionRiskBps << ' ' << r.lossRiskBps << ' '
        << r.eventIntervalMinutes << ' ' << (r.enabled ? 1 : 0) << '\n';
    out << "SHIPMENTS " << shipments_.size() << '\n';
    for (const auto& sh : shipments_) {
        out << "SH " << sh.stableId << ' ' << sh.routeStableId << ' ' << sh.vehicleStableId << ' ' << static_cast<int>(sh.state) << ' '
            << sh.departedMinute << ' ' << sh.expectedArrivalMinute << ' ' << sh.nextEventMinute << ' ' << sh.delayMinutes << ' '
            << sh.interruptionUntilMinute << ' ' << sh.eventOrdinal << ' ' << (sh.promotedDetailed ? 1 : 0) << ' '
            << sh.manifest.size() << ' ' << sh.namedItems.size() << '\n';
        for (const auto& e : sh.manifest) out << "SM " << e.itemId << ' ' << e.count << ' ' << e.unitMassGrams << ' ' << e.unitVolumeMl << '\n';
        for (const auto& e : sh.namedItems) out << "SNI " << e.stableId << ' ' << e.itemId << ' ' << e.massGrams << ' ' << e.volumeMl << '\n';
    }
    out << "EVENTS " << events_.size() << '\n';
    for (const auto& e : events_) out << "EV " << e.stableId << ' ' << static_cast<int>(e.type) << ' ' << e.minute << ' ' << e.shipmentStableId << ' '
        << e.routeStableId << ' ' << e.siteId << ' ' << e.itemId << ' ' << e.quantity << ' ' << e.delayMinutes << '\n';
    out << "END\n";
    return out.str();
}

bool StrategicSimulation::restoreState(std::string_view text, std::string* error) {
    auto fail = [&](const char* message) {
        if (error) *error = message;
        return false;
    };
    std::istringstream in{std::string(text)};
    std::string tag;
    int schema{};
    if (!(in >> tag >> schema) || tag != "ELYSIUM_STRATEGIC" || (schema != 1 && schema != 2))
        return fail("unsupported strategic state header");
    std::uint64_t seed{}, nextShipment{}, nextEvent{};
    StrategicMinute minute{};
    if (!(in >> tag >> seed >> minute >> nextShipment >> nextEvent) || tag != "META" || minute < 0 || nextShipment == 0 || nextEvent == 0)
        return fail("invalid strategic META record");
    StrategicSimulation restored(seed);
    restored.currentMinute_ = minute;
    restored.nextShipmentSerial_ = nextShipment;
    restored.nextEventSerial_ = nextEvent;

    std::size_t siteCount{};
    if (!(in >> tag >> siteCount) || tag != "SITES" || siteCount > 100000) return fail("invalid strategic site count");
    for (std::size_t si = 0; si < siteCount; ++si) {
        StrategicSiteState s{};
        int mode{};
        std::size_t stockN{}, itemN{}, figN{}, cohortN{}, prodN{}, consN{};
        if (!(in >> tag >> s.siteId >> s.systemId >> s.planetId >> mode >> s.maxMassGrams >> s.maxVolumeMl >> s.storedEnergyUnits >>
              s.energyCapacityUnits >> s.infrastructureConditionBps >> stockN >> itemN >> figN >> cohortN >> prodN >> consN) ||
            tag != "SITE" || mode < 0 || mode > 1)
            return fail("invalid strategic SITE record");
        s.mode = static_cast<StrategicSiteMode>(mode);
        for (std::size_t i = 0; i < stockN; ++i) {
            StrategicStockEntry e{};
            if (!(in >> tag >> e.itemId >> e.count >> e.unitMassGrams >> e.unitVolumeMl) || tag != "ST") return fail("invalid ST record");
            s.stock.push_back(e);
        }
        for (std::size_t i = 0; i < itemN; ++i) {
            StrategicNamedItem e{};
            if (!(in >> tag >> e.stableId >> e.itemId >> e.massGrams >> e.volumeMl) || tag != "NI") return fail("invalid NI record");
            s.namedItems.push_back(e);
        }
        for (std::size_t i = 0; i < figN; ++i) {
            StrategicNamedFigure e{}; int alive{};
            if (!(in >> tag >> e.stableId >> e.roleId >> alive) || tag != "NF" || (alive != 0 && alive != 1)) return fail("invalid NF record");
            e.alive = alive != 0; s.namedFigures.push_back(e);
        }
        for (std::size_t i = 0; i < cohortN; ++i) {
            StrategicPopulationCohort e{};
            if (!(in >> tag >> e.stableId >> e.population >> e.laborAvailabilityBps >> e.cohortTypeId) || tag != "CO") return fail("invalid CO record");
            s.cohorts.push_back(e);
        }
        for (std::size_t i = 0; i < prodN; ++i) {
            StrategicProductionRule e{}; std::size_t inputN{};
            if (!(in >> tag >> e.stableId >> e.outputItemId >> e.outputCount >> e.outputUnitMassGrams >> e.outputUnitVolumeMl >>
                  e.batchesPerHour >> e.efficiencyRemainder >> inputN) || tag != "PR" || inputN > 64) return fail("invalid PR record");
            for (std::size_t j = 0; j < inputN; ++j) {
                StrategicIngredient ingredient{};
                if (!(in >> ingredient.itemId >> ingredient.count)) return fail("invalid production ingredient");
                e.inputs.push_back(ingredient);
            }
            s.production.push_back(std::move(e));
        }
        for (std::size_t i = 0; i < consN; ++i) {
            StrategicConsumptionRule e{};
            if (!(in >> tag >> e.stableId >> e.itemId >> e.milliUnitsPerPersonPerDay >> e.fractionalRemainder) || tag != "CR") return fail("invalid CR record");
            s.consumption.push_back(e);
        }
        if (!restored.addSite(std::move(s), error)) return false;
    }

    std::size_t routeCount{};
    if (!(in >> tag >> routeCount) || tag != "ROUTES" || routeCount > 100000) return fail("invalid strategic route count");
    for (std::size_t i = 0; i < routeCount; ++i) {
        StrategicRoute r{}; int type{}, enabled{};
        if (!(in >> tag >> r.stableId >> type >> r.fromSiteId >> r.toSiteId >> r.travelMinutes >> r.maxMassGrams >> r.maxVolumeMl >>
              r.fuelItemId >> r.fuelPerDeparture >> r.energyPerDeparture >> r.hazardRiskBps >> r.inspectionRiskBps >> r.lossRiskBps >>
              r.eventIntervalMinutes >> enabled) ||
            tag != "RT" || type < 0 || type > 4 || (enabled != 0 && enabled != 1)) return fail("invalid RT record");
        r.type = static_cast<StrategicRouteType>(type); r.enabled = enabled != 0;
        if (!restored.addRoute(r, error)) return false;
    }

    std::size_t shipmentCount{};
    if (!(in >> tag >> shipmentCount) || tag != "SHIPMENTS" || shipmentCount > 1000000) return fail("invalid shipment count");
    std::unordered_set<std::uint64_t> shipmentIds;
    for (std::size_t i = 0; i < shipmentCount; ++i) {
        StrategicShipment sh{}; int state{}, promoted{}; std::size_t manifestN{}, namedN{};
        bool parsedShipment = false;
        if (schema == 1) {
            parsedShipment = static_cast<bool>(in >> tag >> sh.stableId >> sh.routeStableId >> sh.vehicleStableId >> state >>
                sh.departedMinute >> sh.expectedArrivalMinute >> sh.nextEventMinute >> sh.delayMinutes >> sh.eventOrdinal >>
                promoted >> manifestN >> namedN);
            sh.interruptionUntilMinute = 0;
        } else {
            parsedShipment = static_cast<bool>(in >> tag >> sh.stableId >> sh.routeStableId >> sh.vehicleStableId >> state >>
                sh.departedMinute >> sh.expectedArrivalMinute >> sh.nextEventMinute >> sh.delayMinutes >> sh.interruptionUntilMinute >>
                sh.eventOrdinal >> promoted >> manifestN >> namedN);
        }
        if (!parsedShipment || tag != "SH" ||
            sh.stableId == 0 || !shipmentIds.insert(sh.stableId).second || state < 0 || state > 6 || (promoted != 0 && promoted != 1) || !restored.route(sh.routeStableId))
            return fail("invalid SH record");
        sh.state = static_cast<StrategicShipmentState>(state); sh.promotedDetailed = promoted != 0;
        for (std::size_t j = 0; j < manifestN; ++j) {
            StrategicStockEntry e{};
            if (!(in >> tag >> e.itemId >> e.count >> e.unitMassGrams >> e.unitVolumeMl) || tag != "SM" || !validStockEntry(e)) return fail("invalid SM record");
            sh.manifest.push_back(e);
        }
        for (std::size_t j = 0; j < namedN; ++j) {
            StrategicNamedItem e{};
            if (!(in >> tag >> e.stableId >> e.itemId >> e.massGrams >> e.volumeMl) || tag != "SNI" || !validNamedItem(e)) return fail("invalid SNI record");
            sh.namedItems.push_back(e);
        }
        normalizeStock(sh.manifest);
        restored.shipments_.push_back(std::move(sh));
    }
    std::sort(restored.shipments_.begin(), restored.shipments_.end(), [](const auto& a, const auto& b){ return a.stableId < b.stableId; });
    std::unordered_set<std::uint64_t> namedOwnership;
    for (const auto& s : restored.sites_)
        for (const auto& item : s.namedItems)
            if (!namedOwnership.insert(item.stableId).second) return fail("named item appears in multiple strategic owners");
    for (const auto& sh : restored.shipments_)
        for (const auto& item : sh.namedItems)
            if (!namedOwnership.insert(item.stableId).second) return fail("named item appears in multiple strategic owners");

    std::size_t eventCount{};
    if (!(in >> tag >> eventCount) || tag != "EVENTS" || eventCount > 5000000) return fail("invalid event count");
    std::unordered_set<std::uint64_t> eventIds;
    for (std::size_t i = 0; i < eventCount; ++i) {
        StrategicEventRecord e{}; int type{};
        if (!(in >> tag >> e.stableId >> type >> e.minute >> e.shipmentStableId >> e.routeStableId >> e.siteId >> e.itemId >> e.quantity >> e.delayMinutes) ||
            tag != "EV" || e.stableId == 0 || !eventIds.insert(e.stableId).second || type < 0 || type > 5)
            return fail("invalid EV record");
        e.type = static_cast<StrategicEventType>(type);
        restored.events_.push_back(e);
    }
    if (!(in >> tag) || tag != "END") return fail("strategic state missing END marker");
    *this = std::move(restored);
    return true;
}

} // namespace elysium

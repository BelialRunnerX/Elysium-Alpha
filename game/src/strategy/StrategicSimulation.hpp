// Intended function: imported strategy implementation for StrategicSimulation; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

using StrategicMinute = std::int64_t;
using SiteId = std::uint64_t;

// Strategic logistics deliberately uses durable IDs and aggregate state only.
// It has no dependency on EnTT, PlanetSurface, renderer handles, or chunk data.
enum class StrategicRouteType : std::uint8_t {
    SurfaceOutpost = 0,
    SurfaceOrbit = 1,
    PlanetPlanet = 2,
    SystemSystem = 3,
    RegionalNetwork = 4
};

enum class StrategicShipmentState : std::uint8_t {
    Loading = 0,
    InTransit = 1,
    AwaitingUnload = 2,
    Interrupted = 3,
    Delivered = 4,
    Lost = 5,
    Cancelled = 6
};

enum class StrategicEventType : std::uint8_t {
    Raid = 0,
    Accident = 1,
    Inspection = 2,
    RouteDelay = 3,
    ShipmentLost = 4,
    DeliveryBlocked = 5
};

enum class StrategicSiteMode : std::uint8_t {
    Strategic = 0,
    Detailed = 1
};

const char* strategicRouteTypeName(StrategicRouteType type);
const char* strategicShipmentStateName(StrategicShipmentState state);
const char* strategicEventTypeName(StrategicEventType type);

struct StrategicStockEntry {
    int itemId{};
    std::int64_t count{};
    std::uint32_t unitMassGrams{1000};
    std::uint32_t unitVolumeMl{1000};

    friend bool operator==(const StrategicStockEntry&, const StrategicStockEntry&) = default;
};

struct StrategicNamedItem {
    std::uint64_t stableId{};
    int itemId{};
    std::uint32_t massGrams{1000};
    std::uint32_t volumeMl{1000};

    friend bool operator==(const StrategicNamedItem&, const StrategicNamedItem&) = default;
};

struct StrategicNamedFigure {
    std::uint64_t stableId{};
    std::uint32_t roleId{};
    bool alive{true};

    friend bool operator==(const StrategicNamedFigure&, const StrategicNamedFigure&) = default;
};

struct StrategicPopulationCohort {
    std::uint64_t stableId{};
    std::uint32_t population{};
    std::uint16_t laborAvailabilityBps{10000};
    std::uint32_t cohortTypeId{};

    friend bool operator==(const StrategicPopulationCohort&, const StrategicPopulationCohort&) = default;
};

struct StrategicIngredient {
    int itemId{};
    std::int64_t count{};

    friend bool operator==(const StrategicIngredient&, const StrategicIngredient&) = default;
};

struct StrategicProductionRule {
    std::uint64_t stableId{};
    int outputItemId{};
    std::int64_t outputCount{1};
    std::uint32_t outputUnitMassGrams{1000};
    std::uint32_t outputUnitVolumeMl{1000};
    std::uint32_t batchesPerHour{};
    std::vector<StrategicIngredient> inputs;
    // Fractional effective-capacity remainder, denominator 10000^2.
    std::uint64_t efficiencyRemainder{};

    friend bool operator==(const StrategicProductionRule&, const StrategicProductionRule&) = default;
};

struct StrategicConsumptionRule {
    std::uint64_t stableId{};
    int itemId{};
    // Thousandths of an item per ordinary cohort member per strategic day.
    std::uint32_t milliUnitsPerPersonPerDay{};
    std::uint64_t fractionalRemainder{};

    friend bool operator==(const StrategicConsumptionRule&, const StrategicConsumptionRule&) = default;
};

struct StrategicShortage {
    int itemId{};
    std::int64_t required{};
    std::int64_t available{};
    std::string reason;

    friend bool operator==(const StrategicShortage&, const StrategicShortage&) = default;
};

struct StrategicSiteState {
    SiteId siteId{};
    std::uint64_t systemId{};
    std::uint64_t planetId{};
    StrategicSiteMode mode{StrategicSiteMode::Strategic};
    std::uint64_t maxMassGrams{};
    std::uint64_t maxVolumeMl{};
    std::uint64_t storedEnergyUnits{};
    std::uint64_t energyCapacityUnits{};
    std::uint16_t infrastructureConditionBps{10000};
    std::vector<StrategicStockEntry> stock;
    std::vector<StrategicNamedItem> namedItems;
    std::vector<StrategicNamedFigure> namedFigures;
    std::vector<StrategicPopulationCohort> cohorts;
    std::vector<StrategicProductionRule> production;
    std::vector<StrategicConsumptionRule> consumption;
    std::vector<StrategicShortage> shortages;
};

struct StrategicRoute {
    std::uint64_t stableId{};
    StrategicRouteType type{StrategicRouteType::SurfaceOutpost};
    SiteId fromSiteId{};
    SiteId toSiteId{};
    StrategicMinute travelMinutes{60};
    std::uint64_t maxMassGrams{};
    std::uint64_t maxVolumeMl{};
    int fuelItemId{};
    std::int64_t fuelPerDeparture{};
    std::uint64_t energyPerDeparture{};
    std::uint16_t hazardRiskBps{};
    std::uint16_t inspectionRiskBps{};
    std::uint16_t lossRiskBps{};
    StrategicMinute eventIntervalMinutes{60};
    bool enabled{true};
};

struct StrategicShipment {
    std::uint64_t stableId{};
    std::uint64_t routeStableId{};
    std::uint64_t vehicleStableId{};
    StrategicShipmentState state{StrategicShipmentState::Loading};
    StrategicMinute departedMinute{};
    StrategicMinute expectedArrivalMinute{};
    StrategicMinute nextEventMinute{};
    StrategicMinute delayMinutes{};
    // While interrupted, remote transit is frozen until this absolute strategic minute.
    // Zero means no active interruption.
    StrategicMinute interruptionUntilMinute{};
    std::uint32_t eventOrdinal{};
    std::vector<StrategicStockEntry> manifest;
    std::vector<StrategicNamedItem> namedItems;
    bool promotedDetailed{};
};

struct StrategicEventRecord {
    std::uint64_t stableId{};
    StrategicEventType type{StrategicEventType::RouteDelay};
    StrategicMinute minute{};
    std::uint64_t shipmentStableId{};
    std::uint64_t routeStableId{};
    SiteId siteId{};
    int itemId{};
    std::int64_t quantity{};
    StrategicMinute delayMinutes{};
};

struct StrategicDependencyEdge {
    std::uint64_t routeStableId{};
    SiteId fromSiteId{};
    SiteId toSiteId{};
    std::uint32_t activeShipmentCount{};
};

struct StrategicSitePromotion {
    SiteId siteId{};
    StrategicMinute strategicMinute{};
    std::vector<StrategicStockEntry> stock;
    std::vector<StrategicNamedItem> namedItems;
    std::vector<StrategicNamedFigure> namedFigures;
    std::vector<StrategicPopulationCohort> cohorts;
};

struct StrategicShipmentPromotion {
    std::uint64_t shipmentStableId{};
    StrategicMinute strategicMinute{};
    StrategicShipmentState state{StrategicShipmentState::InTransit};
    StrategicMinute remainingMinutesUntilArrival{};
    StrategicMinute remainingMinutesUntilEvent{};
    StrategicMinute remainingInterruptionMinutes{};
    std::vector<StrategicStockEntry> manifest;
    std::vector<StrategicNamedItem> namedItems;
};

struct StrategicTelemetry {
    std::uint64_t hourlySiteTicks{};
    std::uint64_t dailyConsumptionTicks{};
    std::uint64_t productionBatches{};
    std::uint64_t shipmentsDispatched{};
    std::uint64_t shipmentsDelivered{};
    std::uint64_t shipmentsLost{};
    std::uint64_t blockedDeliveries{};
    std::uint64_t strategicEvents{};
};

class StrategicSimulation {
public:
    explicit StrategicSimulation(std::uint64_t campaignSeed = 0);

    bool addSite(StrategicSiteState site, std::string* error = nullptr);
    bool addRoute(StrategicRoute route, std::string* error = nullptr);
    bool addNamedItem(SiteId siteId, StrategicNamedItem item, std::string* error = nullptr);
    bool addNamedFigure(SiteId siteId, StrategicNamedFigure figure, std::string* error = nullptr);
    bool addCohort(SiteId siteId, StrategicPopulationCohort cohort, std::string* error = nullptr);
    bool addProductionRule(SiteId siteId, StrategicProductionRule rule, std::string* error = nullptr);
    bool addConsumptionRule(SiteId siteId, StrategicConsumptionRule rule, std::string* error = nullptr);
    bool addStock(SiteId siteId, StrategicStockEntry entry, std::string* error = nullptr);

    const StrategicSiteState* site(SiteId siteId) const;
    StrategicSiteState* site(SiteId siteId);
    const StrategicRoute* route(std::uint64_t stableId) const;
    const StrategicShipment* shipment(std::uint64_t stableId) const;

    // Dispatch transfers aggregate cargo and named items out of the origin at
    // commit time. Capacity/fuel/ownership failures leave origin state intact.
    std::optional<std::uint64_t> dispatch(std::uint64_t routeStableId,
                                          std::uint64_t vehicleStableId,
                                          std::vector<StrategicStockEntry> manifest,
                                          const std::vector<std::uint64_t>& namedItemIds = {},
                                          std::string* error = nullptr);

    // Advances only deterministic strategic boundaries (hourly production,
    // daily consumption, route-event boundaries, arrivals). Calling in many
    // small slices or one large slice yields the same authoritative state.
    bool advanceTo(StrategicMinute targetMinute, std::string* error = nullptr);

    std::optional<StrategicSitePromotion> promoteSite(SiteId siteId, std::string* error = nullptr);
    bool compactSite(const StrategicSitePromotion& snapshot, std::string* error = nullptr);
    std::optional<StrategicShipmentPromotion> promoteShipment(std::uint64_t shipmentStableId, std::string* error = nullptr);
    bool compactShipment(const StrategicShipmentPromotion& snapshot, std::string* error = nullptr);

    std::vector<StrategicDependencyEdge> dependencyGraph(SiteId siteId) const;
    std::string explainSite(SiteId siteId) const;
    std::string explainShipment(std::uint64_t shipmentStableId) const;

    std::string serializeState() const;
    bool restoreState(std::string_view text, std::string* error = nullptr);

    StrategicMinute currentMinute() const { return currentMinute_; }
    const std::vector<StrategicEventRecord>& events() const { return events_; }
    const std::vector<StrategicShipment>& shipments() const { return shipments_; }
    const StrategicTelemetry& telemetry() const { return telemetry_; }

private:
    std::uint64_t campaignSeed_{};
    StrategicMinute currentMinute_{};
    std::uint64_t nextShipmentSerial_{1};
    std::uint64_t nextEventSerial_{1};
    std::vector<StrategicSiteState> sites_;
    std::vector<StrategicRoute> routes_;
    std::vector<StrategicShipment> shipments_;
    std::vector<StrategicEventRecord> events_;
    StrategicTelemetry telemetry_{};

    static void normalizeStock(std::vector<StrategicStockEntry>& stock);
    static std::uint64_t stockMass(const std::vector<StrategicStockEntry>& stock,
                                   const std::vector<StrategicNamedItem>& namedItems = {});
    static std::uint64_t stockVolume(const std::vector<StrategicStockEntry>& stock,
                                     const std::vector<StrategicNamedItem>& namedItems = {});
    static std::int64_t stockCount(const StrategicSiteState& site, int itemId);
    static bool removeStock(StrategicSiteState& site, int itemId, std::int64_t count);
    static bool insertStock(StrategicSiteState& site, StrategicStockEntry entry);
    static std::uint16_t siteLaborAvailabilityBps(const StrategicSiteState& site);

    bool processHour(StrategicMinute hourMinute);
    bool processDay(StrategicMinute dayMinute);
    void processSiteProduction(StrategicSiteState& site);
    void processSiteConsumption(StrategicSiteState& site);
    void processShipmentsAt(StrategicMinute minute);
    void processShipmentEvent(StrategicShipment& shipment, const StrategicRoute& route, StrategicMinute minute);
    void tryUnload(StrategicShipment& shipment, const StrategicRoute& route, StrategicMinute minute);
    void recordEvent(StrategicEventRecord event);
    std::uint64_t allocateShipmentId(std::uint64_t routeStableId);
    std::uint64_t allocateEventId(std::uint64_t shipmentStableId, std::uint32_t ordinal, StrategicEventType type);
    StrategicMinute nextBoundaryAfter(StrategicMinute minute, StrategicMinute target) const;
};

} // namespace elysium

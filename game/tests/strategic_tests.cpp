// Intended function: imported tests implementation for strategic_tests; preserves the agent-authored subsystem contract for later integration/debugging.
#include "strategy/StrategicSimulation.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace elysium;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

std::int64_t countItem(const StrategicSiteState& site, int itemId) {
    std::int64_t count = 0;
    for (const auto& entry : site.stock) if (entry.itemId == itemId) count += entry.count;
    return count;
}

bool hasNamedItem(const StrategicSiteState& site, std::uint64_t stableId) {
    return std::any_of(site.namedItems.begin(), site.namedItems.end(), [&](const auto& item){ return item.stableId == stableId; });
}

StrategicSiteState makeSite(SiteId id, std::uint64_t mass = 100000000, std::uint64_t volume = 100000000) {
    StrategicSiteState site{};
    site.siteId = id;
    site.systemId = 100 + id;
    site.planetId = 200 + id;
    site.maxMassGrams = mass;
    site.maxVolumeMl = volume;
    return site;
}

StrategicRoute makeRoute(std::uint64_t id, SiteId from, SiteId to, StrategicMinute travel = 180) {
    StrategicRoute route{};
    route.stableId = id;
    route.type = StrategicRouteType::SystemSystem;
    route.fromSiteId = from;
    route.toSiteId = to;
    route.travelMinutes = travel;
    route.maxMassGrams = 10000000;
    route.maxVolumeMl = 10000000;
    route.eventIntervalMinutes = 60;
    return route;
}

void testRemoteProductionConsumptionAndDiagnostics() {
    StrategicSimulation sim(0x36A6EULL);
    auto site = makeSite(1);
    site.infrastructureConditionBps = 8000;
    site.stock = {
        {10, 100, 1000, 1000}, // raw input
        {20, 20, 500, 500}     // food
    };
    site.cohorts.push_back({101, 10, 5000, 1});
    site.namedFigures.push_back({201, 7, true});
    StrategicProductionRule production{};
    production.stableId = 301;
    production.outputItemId = 11;
    production.outputCount = 1;
    production.outputUnitMassGrams = 900;
    production.outputUnitVolumeMl = 900;
    production.batchesPerHour = 10;
    production.inputs = {{10, 1}};
    site.production.push_back(production);
    site.consumption.push_back({401, 20, 500, 0}); // 0.5 food/person/day
    require(sim.addSite(site), "failed to add strategic production site");

    require(sim.advanceTo(60), "failed first strategic hour");
    const auto* afterHour = sim.site(1);
    require(afterHour && countItem(*afterHour, 10) == 96 && countItem(*afterHour, 11) == 4,
            "remote production did not apply capacity x labor x infrastructure at hourly boundary");
    require(sim.telemetry().productionBatches == 4, "remote production telemetry incorrect");

    require(sim.advanceTo(1440), "failed first strategic day");
    const auto* afterDay = sim.site(1);
    require(afterDay && countItem(*afterDay, 20) == 15,
            "daily remote population consumption did not include named figure separately from cohort");

    StrategicSimulation blocked(7);
    auto blockedSite = makeSite(9);
    blockedSite.production.push_back({901, 99, 1, 1000, 1000, 1, {{98, 2}}, 0});
    require(blocked.addSite(blockedSite), "failed blocked production fixture");
    require(blocked.advanceTo(60), "failed blocked production hour");
    const auto* b = blocked.site(9);
    require(b && !b->shortages.empty() && b->shortages.front().itemId == 98,
            "remote shortage did not expose missing production dependency");
    require(blocked.explainSite(9).find("Shortages") != std::string::npos,
            "site Why-style explanation omitted shortage reason");
}

void testFreightDispatchCapacityFuelAndNamedIdentity() {
    StrategicSimulation sim(0xF1E1AULL);
    auto source = makeSite(10);
    source.stock = {
        {1, 50, 1000, 1000},
        {99, 5, 100, 100}
    };
    source.namedItems.push_back({7001, 500, 2000, 1500});
    source.energyCapacityUnits = 100;
    source.storedEnergyUnits = 100;
    require(sim.addSite(source), "failed freight source site");
    require(sim.addSite(makeSite(11)), "failed freight destination site");

    auto route = makeRoute(1001, 10, 11, 180);
    route.type = StrategicRouteType::SystemSystem;
    route.fuelItemId = 99;
    route.fuelPerDeparture = 2;
    route.energyPerDeparture = 25;
    require(sim.addRoute(route), "failed freight route");

    std::string error;
    const auto shipmentId = sim.dispatch(1001, 8001, {{1, 12, 1000, 1000}}, {7001}, &error);
    require(shipmentId.has_value(), "freight dispatch failed: " + error);
    const auto* origin = sim.site(10);
    require(origin && countItem(*origin, 1) == 38 && countItem(*origin, 99) == 3 && origin->storedEnergyUnits == 75 &&
            !hasNamedItem(*origin, 7001),
            "dispatch did not atomically reserve manifest, fuel/energy and named item from origin");

    require(sim.advanceTo(180), "failed freight transit");
    const auto* shipment = sim.shipment(*shipmentId);
    const auto* destination = sim.site(11);
    require(shipment && shipment->state == StrategicShipmentState::Delivered,
            "zero-risk freight did not deliver at route travel time");
    require(destination && countItem(*destination, 1) == 12 && hasNamedItem(*destination, 7001),
            "delivered aggregate/named cargo did not reconcile exactly");
    require(sim.telemetry().shipmentsDelivered == 1, "shipment delivery telemetry incorrect");
}

void testBlockedUnloadAndSitePromotionReconciliation() {
    StrategicSimulation sim(0xB10CCEDULL);
    auto source = makeSite(20);
    source.stock = {{5, 20, 1000, 1000}};
    require(sim.addSite(source), "failed blocked source");
    auto destination = makeSite(21, 5000, 5000);
    destination.stock = {{6, 5, 1000, 1000}}; // completely full
    require(sim.addSite(destination), "failed blocked destination");
    require(sim.addRoute(makeRoute(2001, 20, 21, 60)), "failed blocked route");
    const auto id = sim.dispatch(2001, 0, {{5, 3, 1000, 1000}});
    require(id.has_value(), "blocked-unload dispatch failed");
    require(sim.advanceTo(60), "failed blocked arrival");
    require(sim.shipment(*id)->state == StrategicShipmentState::AwaitingUnload &&
            sim.shipment(*id)->manifest.size() == 1 && sim.shipment(*id)->manifest.front().count == 3,
            "saturated destination consumed or deleted queued cargo");

    auto promoted = sim.promoteSite(21);
    require(promoted && promoted->stock.size() == 1 && promoted->stock.front().count == 5,
            "site promotion did not expose exact summarized stock");
    promoted->stock.clear();
    promoted->namedItems.push_back({9001, 77, 500, 500});
    require(sim.advanceTo(180), "strategic clock failed while site was detailed");
    require(sim.compactSite(*promoted), "site compaction failed");
    const auto* reconciled = sim.site(21);
    require(reconciled && reconciled->mode == StrategicSiteMode::Strategic && countItem(*reconciled, 6) == 0 && hasNamedItem(*reconciled, 9001),
            "detailed site snapshot did not reconcile aggregate and named state exactly");

    // Retry occurs at a fixed strategic boundary after compaction.
    require(sim.advanceTo(240), "blocked delivery retry failed");
    require(sim.shipment(*id)->state == StrategicShipmentState::Delivered && countItem(*sim.site(21), 5) == 3,
            "blocked shipment did not unload after destination capacity became available");
}

void testShipmentPromotionFreezesStrategicTransit() {
    StrategicSimulation sim(0x36ULL);
    auto source = makeSite(30);
    source.stock = {{8, 10, 1000, 1000}};
    source.namedItems.push_back({36001, 81, 100, 100});
    require(sim.addSite(source), "failed shipment promotion source");
    require(sim.addSite(makeSite(31)), "failed shipment promotion destination");
    require(sim.addRoute(makeRoute(3001, 30, 31, 600)), "failed long freight route");
    const auto id = sim.dispatch(3001, 36002, {{8, 4, 1000, 1000}}, {36001});
    require(id.has_value(), "failed long shipment dispatch");
    require(sim.advanceTo(60), "failed pre-promotion advance");
    auto promoted = sim.promoteShipment(*id);
    require(promoted && promoted->remainingMinutesUntilArrival == 540 && promoted->namedItems.size() == 1,
            "shipment promotion did not preserve remaining route state and named cargo");

    require(sim.advanceTo(660), "failed strategic advance around promoted shipment");
    require(sim.shipment(*id)->promotedDetailed && sim.shipment(*id)->state == StrategicShipmentState::InTransit,
            "promoted detailed shipment advanced remotely behind active simulation");
    require(sim.compactShipment(*promoted), "shipment compaction failed");
    require(sim.advanceTo(1200), "failed post-compaction transit");
    require(sim.shipment(*id)->state == StrategicShipmentState::Delivered && hasNamedItem(*sim.site(31), 36001),
            "compacted shipment did not resume with exact aggregate/named cargo");
}

StrategicSimulation makeDeterminismFixture() {
    StrategicSimulation sim(0xD3715EEDULL);
    auto a = makeSite(40);
    a.stock = {{1, 200, 1000, 1000}, {2, 20, 100, 100}, {3, 100, 500, 500}};
    a.cohorts.push_back({401, 25, 7200, 1});
    a.production.push_back({402, 4, 2, 800, 800, 5, {{1, 1}}, 0});
    a.consumption.push_back({403, 3, 250, 0});
    sim.addSite(a);
    sim.addSite(makeSite(41));
    auto r = makeRoute(4001, 40, 41, 720);
    r.type = StrategicRouteType::RegionalNetwork;
    r.fuelItemId = 2;
    r.fuelPerDeparture = 1;
    r.hazardRiskBps = 8000;
    r.inspectionRiskBps = 6500;
    r.lossRiskBps = 2000;
    r.eventIntervalMinutes = 120;
    sim.addRoute(r);
    const auto id = sim.dispatch(4001, 44001, {{1, 40, 1000, 1000}});
    require(id.has_value(), "determinism fixture dispatch failed");
    return sim;
}

void testStrategicDeterminismEventsPersistenceAndDependencyGraph() {
    auto oneShot = makeDeterminismFixture();
    auto sliced = makeDeterminismFixture();
    require(oneShot.advanceTo(2880), "one-shot strategic advance failed");
    for (StrategicMinute t = 15; t <= 2880; t += 15) require(sliced.advanceTo(t), "sliced strategic advance failed");

    require(oneShot.serializeState() == sliced.serializeState(),
            "fixed state/seed produced different strategic outcome under different time slicing");
    require(!oneShot.events().empty(), "high-risk strategic route produced no persistent route events");

    const auto deps = oneShot.dependencyGraph(40);
    require(deps.size() == 1 && deps.front().routeStableId == 4001,
            "strategic logistics dependency graph omitted route");

    StrategicSimulation restored;
    std::string error;
    const auto encoded = oneShot.serializeState();
    require(restored.restoreState(encoded, &error), "strategic state round-trip failed: " + error);
    require(restored.serializeState() == encoded,
            "strategic persistence codec changed stable IDs/manifests/events on round trip");
}


void testStrategicInterruptionsAndLegacySchema() {
    StrategicSimulation sim(0xA36A36ULL);
    auto source = makeSite(60);
    source.stock = {{1, 20, 1000, 1000}};
    require(sim.addSite(source), "failed interruption source");
    require(sim.addSite(makeSite(61)), "failed interruption destination");
    auto route = makeRoute(6001, 60, 61, 600);
    route.hazardRiskBps = 10000;
    route.inspectionRiskBps = 0;
    route.lossRiskBps = 0;
    route.eventIntervalMinutes = 60;
    require(sim.addRoute(route), "failed interruption route");
    const auto id = sim.dispatch(6001, 66001, {{1, 8, 1000, 1000}});
    require(id.has_value(), "failed interruption shipment dispatch");

    require(sim.advanceTo(60), "failed first interruption boundary");
    const auto* interrupted = sim.shipment(*id);
    require(interrupted && interrupted->state == StrategicShipmentState::Interrupted &&
            interrupted->interruptionUntilMinute > sim.currentMinute(),
            "hazard delay did not become an explicit persistent interruption state");
    const auto until = interrupted->interruptionUntilMinute;
    const auto explain = sim.explainShipment(*id);
    require(explain.find("Interrupted") != std::string::npos &&
            explain.find("interrupted until minute") != std::string::npos,
            "shipment Why-style explanation omitted interruption state");

    auto promoted = sim.promoteShipment(*id);
    require(promoted && promoted->state == StrategicShipmentState::Interrupted &&
            promoted->remainingInterruptionMinutes == until - 60,
            "promotion did not preserve remaining interruption time");
    require(sim.advanceTo(until + 300), "strategic clock failed while interrupted shipment detailed");
    require(sim.compactShipment(*promoted), "failed interruption compaction");
    const auto* compacted = sim.shipment(*id);
    require(compacted && compacted->state == StrategicShipmentState::Interrupted &&
            compacted->interruptionUntilMinute == sim.currentMinute() + promoted->remainingInterruptionMinutes,
            "compaction did not rebase interruption deadline from detailed simulation");
    const auto resumedAt = compacted->interruptionUntilMinute;
    require(sim.advanceTo(resumedAt), "failed interruption resume boundary");
    require(sim.shipment(*id)->state == StrategicShipmentState::InTransit,
            "interrupted strategic shipment did not resume at deterministic boundary");

    // Schema 1 had no interruption deadline. It must remain readable so subsystem
    // integration can preserve old saves while schema 2 carries the richer state.
    const std::string legacy =
        "ELYSIUM_STRATEGIC 1\n"
        "META 1 0 2 1\n"
        "SITES 2\n"
        "SITE 1 101 201 0 100000 100000 0 0 10000 0 0 0 0 0 0\n"
        "SITE 2 102 202 0 100000 100000 0 0 10000 0 0 0 0 0 0\n"
        "ROUTES 1\n"
        "RT 10 3 1 2 60 100000 100000 0 0 0 0 0 0 60 1\n"
        "SHIPMENTS 1\n"
        "SH 123 10 0 1 0 60 60 0 0 0 0 0\n"
        "EVENTS 0\n"
        "END\n";
    StrategicSimulation restored;
    std::string error;
    require(restored.restoreState(legacy, &error), "schema-1 strategic state no longer readable: " + error);
    require(restored.shipment(123) && restored.shipment(123)->interruptionUntilMinute == 0,
            "legacy shipment gained a fabricated interruption deadline");
    require(restored.serializeState().find("ELYSIUM_STRATEGIC 2") == 0,
            "legacy strategic state did not migrate to canonical schema 2 on write");
}

void testRouteCapacityFailureIsAtomic() {
    StrategicSimulation sim(123);
    auto source = makeSite(50);
    source.stock = {{1, 100, 1000, 1000}, {9, 1, 100, 100}};
    source.namedItems.push_back({50001, 99, 1000, 1000});
    require(sim.addSite(source), "failed atomic dispatch source");
    require(sim.addSite(makeSite(51)), "failed atomic dispatch target");
    auto route = makeRoute(5001, 50, 51, 60);
    route.maxMassGrams = 1000;
    route.maxVolumeMl = 1000;
    route.fuelItemId = 9;
    route.fuelPerDeparture = 1;
    require(sim.addRoute(route), "failed atomic dispatch route");

    std::string error;
    const auto id = sim.dispatch(5001, 0, {{1, 2, 1000, 1000}}, {50001}, &error);
    require(!id && error.find("capacity") != std::string::npos, "over-capacity route dispatch unexpectedly succeeded");
    const auto* after = sim.site(50);
    require(after && countItem(*after, 1) == 100 && countItem(*after, 9) == 1 && hasNamedItem(*after, 50001),
            "failed dispatch partially consumed stock, fuel or named identity");
}

} // namespace

int main() {
    try {
        testRemoteProductionConsumptionAndDiagnostics();
        testFreightDispatchCapacityFuelAndNamedIdentity();
        testBlockedUnloadAndSitePromotionReconciliation();
        testShipmentPromotionFreezesStrategicTransit();
        testStrategicDeterminismEventsPersistenceAndDependencyGraph();
        testStrategicInterruptionsAndLegacySchema();
        testRouteCapacityFailureIsAtomic();
        std::cout << "Elysium strategic tests: PASS\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Elysium strategic tests: FAIL: " << e.what() << '\n';
        return 1;
    }
}

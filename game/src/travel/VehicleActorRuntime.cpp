#include "travel/VehicleActorRuntime.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace elysium::travel {
namespace {
using elysium::safe::nonNegative;

bool finitePositive(double v) { return std::isfinite(v) && v > 0.0; }
std::uint64_t mix(std::uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
std::uint64_t idFor(std::uint64_t seed, std::uint64_t ordinal) {
    auto v = mix(seed ^ mix(ordinal + 1));
    return v == 0 ? ordinal + 1 : v;
}
std::uint64_t cargoCount(const GroundVehicleState& v) {
    std::uint64_t total = 0;
    for (const auto& s : v.cargo) total = elysium::safe::saturatingAdd(total, s.count);
    return total;
}
void normalizeCargo(GroundVehicleState& v) {
    std::sort(v.cargo.begin(), v.cargo.end(), [](const auto& a, const auto& b) {
        if (a.itemStableId != b.itemStableId) return a.itemStableId < b.itemStableId;
        return a.contentId < b.contentId;
    });
    std::vector<VehicleCargoStack> out;
    out.reserve(v.cargo.size());
    for (const auto& s : v.cargo) {
        if (s.itemStableId == 0 || s.contentId == 0 || s.count == 0) continue;
        if (!out.empty() && out.back().itemStableId == s.itemStableId && out.back().contentId == s.contentId)
            out.back().count = elysium::safe::saturatingAdd(out.back().count, s.count);
        else out.push_back(s);
    }
    v.cargo = std::move(out);
}
}

bool VehicleActorRuntime::publish(VehicleArchetypeDefinition d) {
    if (!d.contentId || !finitePositive(d.cruiseSpeedMps)) return false;
    d.energyCapacity = nonNegative(d.energyCapacity);
    d.fuelCapacity = nonNegative(d.fuelCapacity);
    d.maximumSlopeDegrees = nonNegative(d.maximumSlopeDegrees, 90.0);
    d.maximumRoughness = nonNegative(d.maximumRoughness);
    d.maximumWaterDepthMeters = nonNegative(d.maximumWaterDepthMeters);
    d.baseSignaturePerKm = nonNegative(d.baseSignaturePerKm);
    archetypes_[d.contentId] = d;
    return true;
}

bool VehicleActorRuntime::publish(VehicleModuleDefinition d) {
    if (!d.contentId || !std::isfinite(d.speedMultiplier) || !std::isfinite(d.signatureMultiplier) || d.speedMultiplier < 0.0 || d.signatureMultiplier < 0.0) return false;
    d.energyCapacityBonus = nonNegative(d.energyCapacityBonus);
    d.fuelCapacityBonus = nonNegative(d.fuelCapacityBonus);
    d.poweredEnergyPerKm = nonNegative(d.poweredEnergyPerKm);
    d.speedMultiplier = nonNegative(d.speedMultiplier);
    d.signatureMultiplier = nonNegative(d.signatureMultiplier);
    modules_[d.contentId] = d;
    return true;
}

bool VehicleActorRuntime::create(GroundVehicleState s) {
    if (!s.stableId || !s.archetypeContentId || !archetypes_.contains(s.archetypeContentId) || vehicles_.contains(s.stableId)) return false;
    s.energy = nonNegative(s.energy);
    s.fuel = nonNegative(s.fuel);
    s.hull = nonNegative(s.hull);
    normalizeCargo(s);
    vehicles_.emplace(s.stableId, std::move(s));
    return true;
}

bool VehicleActorRuntime::installModules(std::uint64_t id, std::vector<InstalledVehicleModule> mods) {
    auto it = vehicles_.find(id); if (it == vehicles_.end()) return false;
    std::sort(mods.begin(), mods.end(), [](const auto& a, const auto& b){ return a.stableItemId < b.stableItemId; });
    std::uint64_t last = 0;
    for (const auto& m : mods) {
        if (!m.stableItemId || !m.moduleContentId || !modules_.contains(m.moduleContentId) || m.stableItemId == last) return false;
        last = m.stableItemId;
    }
    it->second.modules = std::move(mods);
    it->second.revision = elysium::safe::saturatingIncrement(it->second.revision);
    return true;
}

const GroundVehicleState* VehicleActorRuntime::find(std::uint64_t id) const {
    auto it = vehicles_.find(id); return it == vehicles_.end() ? nullptr : &it->second;
}

VehicleDerivedState VehicleActorRuntime::derive(std::uint64_t id) const {
    VehicleDerivedState out;
    const auto* v = find(id); if (!v) return out;
    const auto ai = archetypes_.find(v->archetypeContentId); if (ai == archetypes_.end()) return out;
    out.valid = true;
    out.capabilities = ai->second.capabilities;
    out.cargoCapacity = ai->second.cargoCapacity;
    out.energyCapacity = ai->second.energyCapacity;
    out.fuelCapacity = ai->second.fuelCapacity;
    out.cruiseSpeedMps = ai->second.cruiseSpeedMps;
    out.signaturePerKm = ai->second.baseSignaturePerKm;
    out.cargoCount = cargoCount(*v);
    std::vector<InstalledVehicleModule> mods = v->modules;
    std::sort(mods.begin(), mods.end(), [](const auto& a, const auto& b){ return a.stableItemId < b.stableItemId; });
    std::uint64_t last = 0;
    for (const auto& installed : mods) {
        if (!installed.enabled) continue;
        if (!installed.stableItemId || installed.stableItemId == last) { out.valid = false; return out; }
        last = installed.stableItemId;
        auto mi = modules_.find(installed.moduleContentId); if (mi == modules_.end()) { out.valid = false; return out; }
        const auto& m = mi->second;
        out.capabilities |= m.addCapabilities;
        out.cargoCapacity = elysium::safe::saturatingAdd(out.cargoCapacity, m.cargoCapacityBonus);
        out.energyCapacity = nonNegative(out.energyCapacity + m.energyCapacityBonus);
        out.fuelCapacity = nonNegative(out.fuelCapacity + m.fuelCapacityBonus);
        out.cruiseSpeedMps = nonNegative(out.cruiseSpeedMps * m.speedMultiplier);
        out.signaturePerKm = nonNegative(out.signaturePerKm * m.signatureMultiplier);
        out.poweredEnergyPerKm = nonNegative(out.poweredEnergyPerKm + m.poweredEnergyPerKm);
    }
    return out;
}

VehicleTraversalCheck VehicleActorRuntime::checkTraversal(std::uint64_t id, const VehicleTraversalContext& c) const {
    VehicleTraversalCheck out;
    const auto* v = find(id); if (!v) { out.reason = VehicleBlockReason::MissingVehicle; return out; }
    const auto ai = archetypes_.find(v->archetypeContentId); if (ai == archetypes_.end()) { out.reason = VehicleBlockReason::UnknownArchetype; return out; }
    if (v->worldId != c.worldId) { out.reason = VehicleBlockReason::WrongWorld; return out; }
    if (!std::isfinite(c.distanceMeters) || c.distanceMeters < 0.0) { out.reason = VehicleBlockReason::InvalidDistance; return out; }
    const auto d = derive(id); if (!d.valid) { out.reason = VehicleBlockReason::UnknownModule; return out; }
    if (v->driverStableId == 0 && (d.capabilities & vehicleCapability(VehicleCapability::Autonomous)) == 0) { out.reason = VehicleBlockReason::NoDriverOrAutonomy; return out; }
    const bool hover = (d.capabilities & vehicleCapability(VehicleCapability::RoughTerrainHover)) != 0;
    const bool amphibious = (d.capabilities & vehicleCapability(VehicleCapability::Amphibious)) != 0;
    if (!hover && nonNegative(c.slopeDegrees, 90.0) > ai->second.maximumSlopeDegrees) { out.reason = VehicleBlockReason::SlopeUnsupported; return out; }
    if (!hover && nonNegative(c.roughness) > ai->second.maximumRoughness) { out.reason = VehicleBlockReason::RoughnessUnsupported; return out; }
    if (!amphibious && nonNegative(c.waterDepthMeters) > ai->second.maximumWaterDepthMeters) { out.reason = VehicleBlockReason::WaterUnsupported; return out; }
    if (d.cargoCount > d.cargoCapacity) { out.reason = VehicleBlockReason::CargoCapacityExceeded; return out; }
    const double km = nonNegative(c.distanceMeters) / 1000.0;
    out.energyRequired = nonNegative(d.poweredEnergyPerKm * km);
    out.fuelRequired = nonNegative(km * 0.05);
    if (v->energy + 1e-9 < out.energyRequired) { out.reason = VehicleBlockReason::InsufficientEnergy; return out; }
    if (v->fuel + 1e-9 < out.fuelRequired) { out.reason = VehicleBlockReason::InsufficientFuel; return out; }
    const double terrain = hover ? 1.0 : std::clamp(1.0 - nonNegative(c.roughness) * 0.35 - nonNegative(c.slopeDegrees,90.0) / 180.0, 0.1, 1.0);
    out.effectiveSpeedMps = nonNegative(d.cruiseSpeedMps * terrain);
    out.standingSignal = nonNegative(d.signaturePerKm * km);
    out.allowed = true;
    out.reason = VehicleBlockReason::None;
    return out;
}

bool VehicleActorRuntime::commitTraversal(std::uint64_t id, const VehicleTraversalContext& c, VehicleStandingEmission* emission) {
    auto it = vehicles_.find(id); if (it == vehicles_.end()) return false;
    const auto check = checkTraversal(id, c); if (!check.allowed) return false;
    it->second.energy = nonNegative(it->second.energy - check.energyRequired);
    it->second.fuel = nonNegative(it->second.fuel - check.fuelRequired);
    it->second.locationKey = c.destinationKey;
    it->second.revision = elysium::safe::saturatingIncrement(it->second.revision);
    if (emission) *emission = VehicleStandingEmission{id, c.worldId, c.destinationKey, check.standingSignal, it->second.archetypeContentId};
    return true;
}

std::uint64_t VehicleActorRuntime::insertCargo(std::uint64_t id, VehicleCargoStack stack) {
    auto it = vehicles_.find(id); if (it == vehicles_.end() || !stack.itemStableId || !stack.contentId || !stack.count) return 0;
    auto d = derive(id); if (!d.valid || d.cargoCount >= d.cargoCapacity) return 0;
    const auto room = d.cargoCapacity - d.cargoCount;
    const auto accepted = std::min(room, stack.count);
    stack.count = accepted;
    it->second.cargo.push_back(stack);
    normalizeCargo(it->second);
    it->second.revision = elysium::safe::saturatingIncrement(it->second.revision);
    return accepted;
}

std::uint64_t VehicleActorRuntime::extractCargo(std::uint64_t id, std::uint64_t itemStableId, std::uint64_t count) {
    auto it = vehicles_.find(id); if (it == vehicles_.end() || !itemStableId || !count) return 0;
    normalizeCargo(it->second);
    auto si = std::find_if(it->second.cargo.begin(), it->second.cargo.end(), [&](const auto& s){ return s.itemStableId == itemStableId; });
    if (si == it->second.cargo.end()) return 0;
    const auto taken = std::min(count, si->count);
    si->count -= taken;
    if (si->count == 0) it->second.cargo.erase(si);
    it->second.revision = elysium::safe::saturatingIncrement(it->second.revision);
    return taken;
}

VehicleSnapshot VehicleActorRuntime::snapshot() const {
    VehicleSnapshot s;
    for (const auto& [_, v] : archetypes_) s.archetypes.push_back(v);
    for (const auto& [_, v] : modules_) s.modules.push_back(v);
    for (const auto& [_, v] : vehicles_) s.vehicles.push_back(v);
    return s;
}

bool VehicleActorRuntime::restore(const VehicleSnapshot& s) {
    VehicleActorRuntime staged;
    for (const auto& v : s.archetypes) if (!staged.publish(v)) return false;
    for (const auto& v : s.modules) if (!staged.publish(v)) return false;
    for (auto v : s.vehicles) {
        auto mods = v.modules; v.modules.clear();
        if (!staged.create(v) || !staged.installModules(v.stableId, std::move(mods))) return false;
    }
    *this = std::move(staged);
    return true;
}

std::vector<VehicleArchetypeDefinition> makeDefaultVehicleArchetypes(std::uint64_t seed) {
    const auto cap = vehicleCapability;
    return {
        {idFor(seed,0), cap(VehicleCapability::Survey), 8, 120, 100, 18, 35, .7, .35, .05},
        {idFor(seed,1), cap(VehicleCapability::BulkCargo), 64, 220, 180, 7, 20, .45, .25, .12},
        {idFor(seed,2), cap(VehicleCapability::MobileMining)|cap(VehicleCapability::BulkCargo), 32, 300, 220, 4.5, 16, .35, .20, .45},
        {idFor(seed,3), cap(VehicleCapability::Amphibious), 20, 160, 140, 13, 18, .6, 1000000, .10},
        {idFor(seed,4), cap(VehicleCapability::RoughTerrainHover), 18, 240, 180, 22, 90, 1000000, 1000000, .18},
        {idFor(seed,5), cap(VehicleCapability::SiegeLogistics)|cap(VehicleCapability::BulkCargo), 96, 360, 320, 5.5, 22, .5, .30, .50},
    };
}

} // namespace elysium::travel

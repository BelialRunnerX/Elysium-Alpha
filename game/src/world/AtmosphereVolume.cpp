// Intended function: pure bounded atmosphere exchange; topology authority supplied externally.
#include "world/AtmosphereVolume.hpp"

#include <algorithm>
#include <map>

namespace elysium {

AtmosphereState sanitizeAtmosphere(AtmosphereState v) noexcept {
    v.pressureKPa = safe::nonNegative(v.pressureKPa);
    v.oxygenFraction = safe::finiteClamp(v.oxygenFraction, 0.0, 1.0);
    v.toxicFraction = safe::finiteClamp(v.toxicFraction, 0.0, 1.0);
    v.smokeFraction = safe::finiteClamp(v.smokeFraction, 0.0, 1.0);
    const double total = v.oxygenFraction + v.toxicFraction + v.smokeFraction;
    if (total > 1.0) {
        v.oxygenFraction /= total;
        v.toxicFraction /= total;
        v.smokeFraction /= total;
    }
    v.temperatureK = safe::nonNegative(v.temperatureK);
    v.gasAmount = safe::nonNegative(v.gasAmount);
    return v;
}

std::vector<AtmosphereDelta> AtmosphereSolver::compute(
    std::span<const AtmosphereState> rawVolumes,
    std::span<const AtmosphereExchange> exchanges,
    double stepSeconds) const {
    stepSeconds = safe::nonNegative(stepSeconds, 60.0);
    std::map<std::uint64_t, AtmosphereState> volumes;
    for (auto v : rawVolumes) if (v.volumeId != 0) volumes[v.volumeId] = sanitizeAtmosphere(v);
    std::map<std::uint64_t, AtmosphereDelta> delta;
    for (const auto& [id, _] : volumes) delta[id].volumeId = id;

    for (auto link : exchanges) {
        auto ia = volumes.find(link.fromVolume), ib = volumes.find(link.toVolume);
        if (ia == volumes.end() || ib == volumes.end() || ia->first == ib->first) continue;
        const auto& a = ia->second; const auto& b = ib->second;
        const double conductance = safe::nonNegative(link.conductance, 1.0);
        const double pressureDiff = safe::finiteClamp(a.pressureKPa - b.pressureKPa, -1.0e6, 1.0e6);
        const double amount = safe::finiteClamp(pressureDiff * conductance * stepSeconds * 0.01, -a.gasAmount, b.gasAmount);
        if (amount == 0.0) continue;
        auto& da = delta[a.volumeId]; auto& db = delta[b.volumeId];
        da.gasDelta = safe::finiteClamp(da.gasDelta - amount, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);
        db.gasDelta = safe::finiteClamp(db.gasDelta + amount, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);
        da.pressureDeltaKPa = safe::finiteClamp(da.pressureDeltaKPa - pressureDiff * conductance * stepSeconds * 0.01, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);
        db.pressureDeltaKPa = safe::finiteClamp(db.pressureDeltaKPa + pressureDiff * conductance * stepSeconds * 0.01, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);

        const AtmosphereState& source = amount > 0.0 ? a : b;
        const double moved = std::abs(amount);
        const double signA = amount > 0.0 ? -1.0 : 1.0;
        da.oxygenDelta += signA * moved * source.oxygenFraction;
        db.oxygenDelta -= signA * moved * source.oxygenFraction;
        da.toxicDelta += signA * moved * source.toxicFraction;
        db.toxicDelta -= signA * moved * source.toxicFraction;
        da.smokeDelta += signA * moved * source.smokeFraction;
        db.smokeDelta -= signA * moved * source.smokeFraction;
    }
    std::vector<AtmosphereDelta> out;
    out.reserve(delta.size());
    for (auto& [_, d] : delta) {
        d.oxygenDelta = safe::finiteClamp(d.oxygenDelta, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);
        d.toxicDelta = safe::finiteClamp(d.toxicDelta, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);
        d.smokeDelta = safe::finiteClamp(d.smokeDelta, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);
        out.push_back(d);
    }
    return out;
}

} // namespace elysium

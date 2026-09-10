#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

bool atmosphereCountsAsSealed(const RoomAtmosphere& room) {
    // Project integration contract: a successful bounded fill is sealed. Sky connection or
    // budget exhaustion means open. Do not infer openness merely from the presence of portals.
    return room.sealed && !room.connectedToSky && !room.boundedFillExhausted;
}

void equalizeAtmosphere(RoomAtmosphere& a, RoomAtmosphere& b, float aperture, float dt) {
    const float rate = saturate(aperture) * std::max(0.0f, dt) * 0.8f;
    if (rate <= 0.0f) return;
    const float pressureDelta = (a.pressure - b.pressure) * rate;
    const float oxygenDelta = (a.oxygen - b.oxygen) * rate * 0.5f;
    const float smokeDelta = (a.smoke - b.smoke) * rate * 0.5f;
    const float toxinDelta = (a.toxins - b.toxins) * rate * 0.5f;
    const float temperatureDelta = (a.temperature - b.temperature) * rate * 0.35f;

    a.pressure = std::max(0.0f, a.pressure - pressureDelta);
    b.pressure = std::max(0.0f, b.pressure + pressureDelta);
    a.oxygen = saturate(a.oxygen - oxygenDelta);
    b.oxygen = saturate(b.oxygen + oxygenDelta);
    a.smoke = saturate(a.smoke - smokeDelta);
    b.smoke = saturate(b.smoke + smokeDelta);
    a.toxins = saturate(a.toxins - toxinDelta);
    b.toxins = saturate(b.toxins + toxinDelta);
    a.temperature = saturate(a.temperature - temperatureDelta);
    b.temperature = saturate(b.temperature + temperatureDelta);
}

void advanceFire(FireState& fire, RoomAtmosphere& room, float flammability, float suppression, float dt) {
    const float step = std::max(0.0f, dt);
    if (fire.intensity <= 0.0f || fire.fuel <= 0.0f || room.oxygen <= 0.01f) {
        fire.intensity = std::max(0.0f, fire.intensity - step * 0.25f);
        return;
    }
    const float oxygenFactor = saturate(room.oxygen / 0.21f);
    const float growth = saturate(flammability) * oxygenFactor * step * 0.28f;
    const float extinguish = saturate(suppression) * step * 0.55f;
    fire.intensity = saturate(fire.intensity + growth - extinguish);
    const float burn = fire.intensity * step * 0.08f;
    fire.fuel = std::max(0.0f, fire.fuel - burn);
    room.oxygen = saturate(room.oxygen - fire.oxygenDemand * fire.intensity * step * 0.01f);
    room.smoke = saturate(room.smoke + fire.smokeRate * fire.intensity * step * 0.025f);
    room.temperature = saturate(room.temperature + fire.heatRate * fire.intensity * step * 0.015f);
}

void advanceContamination(ContaminationState& contamination, float cleaning, float isolation, float dt) {
    const float step = std::max(0.0f, dt);
    const float removal = saturate(cleaning) * (0.08f + saturate(isolation) * 0.06f) * step;
    contamination.intensity = std::max(0.0f, contamination.intensity - removal);
}

} // namespace elysium::fortress

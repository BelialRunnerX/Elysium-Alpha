// Intended function: planet-class operational pressure derived from physical profile; ore fields remain independent.
#include "world/PlanetClassProfile.hpp"

#include "core/Saturating.hpp"

namespace elysium {

PlanetClassDecisionProfile decisionProfile(const PlanetPhysicalProfile& p) noexcept {
    PlanetClassDecisionProfile out{};
    out.planetClass = p.planetClass;
    out.thermalEngineeringPressure = safe::finiteClamp(p.irradiation / (1.0 + p.irradiation), 0.0, 1.0);
    out.corrosionEngineeringPressure = safe::finiteClamp(p.toxicityPotential, 0.0, 1.0);
    out.radiationEngineeringPressure = safe::finiteClamp(p.radiationPotential, 0.0, 1.0);
    out.pressureEngineeringPressure = safe::finiteClamp(p.oceanPotential, 0.0, 1.0);
    out.shelterPressure = 0.0;
    switch (p.planetClass) {
        case PlanetClass::Temperate: out.primaryHazard=PrimaryHazard::None; out.naturallyBreathableCandidate=true; break;
        case PlanetClass::Barren: out.primaryHazard=PrimaryHazard::Vacuum; out.shelterPressure=1.0; break;
        case PlanetClass::Scorched: out.primaryHazard=PrimaryHazard::Thermal; out.shelterPressure=0.45; break;
        case PlanetClass::Frozen: out.primaryHazard=PrimaryHazard::Cryogenic; out.shelterPressure=0.45; break;
        case PlanetClass::Toxic: out.primaryHazard=PrimaryHazard::Corrosive; out.shelterPressure=0.85; break;
        case PlanetClass::Irradiated: out.primaryHazard=PrimaryHazard::Radiological; out.shelterPressure=0.75; break;
        case PlanetClass::Oceanic: out.primaryHazard=PrimaryHazard::Pressure; out.shelterPressure=0.65; break;
        case PlanetClass::Anomalous: out.primaryHazard=PrimaryHazard::Mixed; out.shelterPressure=1.0; out.claimable=false; break;
    }
    out.shelterPressure = safe::finiteClamp(out.shelterPressure, 0.0, 1.0);
    return out;
}

} // namespace elysium

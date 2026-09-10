#include "planet.h"
#include <cmath>

namespace ely {

const char* planetClassName(PlanetClass c) {
    switch (c) {
        case PlanetClass::Barren:     return "Barren";
        case PlanetClass::Temperate:  return "Temperate";
        case PlanetClass::Scorched:   return "Scorched";
        case PlanetClass::Frozen:     return "Frozen";
        case PlanetClass::Toxic:      return "Toxic";
        case PlanetClass::Irradiated: return "Irradiated";
        case PlanetClass::Oceanic:    return "Oceanic";
        case PlanetClass::Anomalous:  return "Anomalous";
    }
    return "?";
}

const char* elementName(Element e) {
    switch (e) {
        case Element::None:        return "none";
        case Element::Void:        return "Void";
        case Element::Plasma:      return "Plasma";
        case Element::Neural:      return "Neural";
        case Element::Dimensional: return "Dimensional";
        case Element::Kinetic:     return "Kinetic";
    }
    return "?";
}

const char* hazardName(Hazard h) {
    switch (h) {
        case Hazard::None:         return "none";
        case Hazard::Vacuum:       return "vacuum";
        case Hazard::Thermal:      return "thermal";
        case Hazard::Cryogenic:    return "cryogenic";
        case Hazard::Corrosive:    return "corrosive";
        case Hazard::Radiological: return "radiological";
        case Hazard::Pressure:     return "pressure";
    }
    return "?";
}

// Mechanism 3: the five elements are also the five hazards.
Element classElement(PlanetClass c) {
    switch (c) {
        case PlanetClass::Scorched:   return Element::Plasma;
        case PlanetClass::Frozen:     return Element::Kinetic;
        case PlanetClass::Toxic:      return Element::Neural;
        case PlanetClass::Irradiated: return Element::Void;
        case PlanetClass::Oceanic:    return Element::Dimensional;
        default:                      return Element::None;
    }
}

Hazard classHazard(PlanetClass c) {
    switch (c) {
        case PlanetClass::Barren:     return Hazard::Vacuum;
        case PlanetClass::Temperate:  return Hazard::None;
        case PlanetClass::Scorched:   return Hazard::Thermal;
        case PlanetClass::Frozen:     return Hazard::Cryogenic;
        case PlanetClass::Toxic:      return Hazard::Corrosive;
        case PlanetClass::Irradiated: return Hazard::Radiological;
        case PlanetClass::Oceanic:    return Hazard::Pressure;
        case PlanetClass::Anomalous:  return Hazard::Radiological;
    }
    return Hazard::None;
}

namespace {

// Class weights, per spec §3.4. Sum is 100.
struct ClassWeight { PlanetClass cls; int weight; };
constexpr ClassWeight kClassWeights[] = {
    {PlanetClass::Barren,     24},
    {PlanetClass::Scorched,   14},
    {PlanetClass::Frozen,     14},
    {PlanetClass::Toxic,      12},
    {PlanetClass::Oceanic,    11},
    {PlanetClass::Irradiated, 10},
    {PlanetClass::Temperate,   9},
    {PlanetClass::Anomalous,   6},
};

PlanetClass pickClass(Rng& rng) {
    int roll = int(rng.below(100));
    for (const auto& cw : kClassWeights) {
        if (roll < cw.weight) return cw.cls;
        roll -= cw.weight;
    }
    return PlanetClass::Barren;
}

}  // namespace

PlanetParams generatePlanetFromSeed(uint64_t seed) {
    PlanetParams p;
    p.seed = seed;
    Rng rng(seed);

    p.cls = pickClass(rng);
    p.radius = rng.range(1200.0, 6400.0);
    p.gravity = rng.range(0.35, 1.6);

    // Tide-locking is common around small cool stars. Represented as a day
    // length of zero, which makes the day/night hazard split a *geographic*
    // decision rather than a temporal one (spec §4.7).
    p.dayLength = rng.chance(0.22) ? 0.0 : rng.range(6.0, 180.0);
    p.axialTilt = rng.range(0.0, 40.0) * 3.14159265358979 / 180.0;

    p.hazardIntensity = (p.cls == PlanetClass::Temperate) ? 0
                      : (p.cls == PlanetClass::Anomalous) ? 1 + int(rng.below(2))
                      : 1 + int(rng.below(3));

    switch (p.cls) {
        case PlanetClass::Oceanic:
            p.seaLevelFrac = rng.range(0.78, 0.97);
            p.floraDensity = 1 + int(rng.below(3));
            p.faunaDensity = 1 + int(rng.below(3));
            break;
        case PlanetClass::Temperate:
            p.seaLevelFrac = rng.range(0.32, 0.62);
            p.floraDensity = 2 + int(rng.below(2));
            p.faunaDensity = 2 + int(rng.below(2));
            break;
        case PlanetClass::Toxic:
            p.seaLevelFrac = rng.range(0.15, 0.55);
            p.floraDensity = 2 + int(rng.below(2));
            p.faunaDensity = 1 + int(rng.below(3));
            break;
        case PlanetClass::Frozen:
            p.seaLevelFrac = rng.range(0.30, 0.70);
            p.floraDensity = int(rng.below(2));
            p.faunaDensity = int(rng.below(3));
            break;
        case PlanetClass::Barren:
        case PlanetClass::Irradiated:
            p.seaLevelFrac = 0.0;
            p.floraDensity = 0;
            p.faunaDensity = (p.cls == PlanetClass::Irradiated) ? int(rng.below(2)) : 0;
            break;
        case PlanetClass::Scorched:
            p.seaLevelFrac = 0.0;
            p.floraDensity = int(rng.below(2));
            p.faunaDensity = int(rng.below(2));
            break;
        case PlanetClass::Anomalous:
            p.seaLevelFrac = rng.range(0.0, 0.8);
            p.floraDensity = int(rng.below(4));
            p.faunaDensity = int(rng.below(4));
            break;
    }

    // Anomalous worlds are never claimable: the register does not recognise
    // them (spec §3.4).
    p.claimable = (p.cls != PlanetClass::Anomalous);

    p.reliefScale = rng.range(0.55, 1.45);
    p.oceanBias = rng.range(-0.18, 0.18);
    return p;
}

PlanetParams generatePlanet(uint64_t systemSeed, uint32_t index) {
    return generatePlanetFromSeed(planetSeed(systemSeed, index));
}

}  // namespace ely

// A planet's generated parameters. Specification §3.4.
//
// Everything here is derived from the planet seed and nothing is stored. The
// whole struct is about 120 bytes and regenerating it costs microseconds,
// which is what lets the galaxy map show thousands of systems while panning
// (principle S1).
#pragma once
#include "../core/seed.h"
#include <cstdint>
#include <string>

namespace ely {

enum class PlanetClass : uint8_t {
    Barren, Temperate, Scorched, Frozen, Toxic, Irradiated, Oceanic, Anomalous
};

enum class Element : uint8_t { None, Void, Plasma, Neural, Dimensional, Kinetic };

enum class Hazard : uint8_t {
    None, Vacuum, Thermal, Cryogenic, Corrosive, Radiological, Pressure
};

const char* planetClassName(PlanetClass c);
const char* elementName(Element e);
const char* hazardName(Hazard h);

// The element a class is aligned with — Mechanism 3 and 4 in one lookup.
Element classElement(PlanetClass c);
Hazard  classHazard(PlanetClass c);

struct PlanetParams {
    uint64_t seed = 0;
    PlanetClass cls = PlanetClass::Barren;

    double radius = 3000.0;      // metres, sea level
    double gravity = 1.0;        // g
    double dayLength = 30.0;     // minutes; 0 == tide-locked
    double axialTilt = 0.0;      // radians
    double seaLevelFrac = 0.4;   // 0 dry .. 1 ocean world
    int    hazardIntensity = 1;  // 0..3
    int    floraDensity = 0;     // 0..3
    int    faunaDensity = 0;     // 0..3
    bool   claimable = true;

    // Terrain shaping, varied per planet so two worlds of the same class do
    // not read as the same world with a different palette.
    double reliefScale = 1.0;    // multiplies terrain height range
    double oceanBias = 0.0;      // shifts continentalness

    Element element() const { return classElement(cls); }
    Hazard  hazard()  const { return classHazard(cls); }

    // Altitude range in metres relative to sea level (spec §4.1).
    static constexpr double kMaxAltitude = 256.0;
    static constexpr double kMinAltitude = -512.0;
};

// Generate a planet from a system seed and an orbital index.
PlanetParams generatePlanet(uint64_t systemSeed, uint32_t index);

// Generate one directly from a planet seed, for tools and tests.
PlanetParams generatePlanetFromSeed(uint64_t planetSeed);

}  // namespace ely

// Intended function: imported world implementation for DestructionPolicy; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/StructuralIntegrity.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace elysium {

enum class TerrainDamageCategory : std::uint8_t {
    LightWeapon,
    MiningTool,
    Explosive,
    VehicleImpact,
    SiegeBreach,
    Megathreat
};

enum class TerrainDamageAction : std::uint8_t {
    None,
    Cosmetic,
    MicroChip,
    MacroBreak,
    Breach
};

struct TerrainDamageRequest {
    SurfaceCellAddress target{};
    TerrainDamageCategory category{TerrainDamageCategory::LightWeapon};
    float energy{};
    // Optional precise micro impact. If omitted, deterministic center-face
    // microcells are used for chipping.
    std::optional<SurfaceMicroAddress> microImpact;
    bool allowNaturalTerrain{true};
    int breachRadius{1};
};

struct TerrainDamageDecision {
    TerrainDamageAction action{TerrainDamageAction::None};
    int microRadius{};
    int macroRadius{};
    bool dirtiesStructure{};
};

struct TerrainDamageResult {
    TerrainDamageDecision primary{};
    int macroCellsRemoved{};
    int microCellsRemoved{};
    int cosmeticMarks{};
    std::vector<SurfaceCellAddress> changedMacroCells;
};

class DestructionPolicy {
public:
    TerrainDamageDecision decide(BlockType material,
                                 TerrainDamageCategory category,
                                 float energy) const;

    TerrainDamageResult apply(PlanetSurface& world,
                              SurfaceStructuralIntegritySystem& structural,
                              const TerrainDamageRequest& request) const;

private:
    static StructuralDamageCause structuralCause(TerrainDamageCategory category);
    static int chipCell(PlanetSurface& world,
                        SurfaceCellAddress cell,
                        const std::optional<SurfaceMicroAddress>& microImpact,
                        int microRadius);
};

} // namespace elysium

// Intended function: imported render implementation for VisualBenchmark; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "render/PresentationAssets.hpp"

#include <array>
#include <cstdint>
#include <string_view>

namespace elysium {

enum class VisualBenchmarkZoneKind : std::uint8_t {
    NaturalForeground = 0,
    FrontierBase,
    ImperialStructure,
    Industry,
    Fauna,
    DestructionTarget,
    LongVista,
    Count
};

struct VisualBenchmarkZoneReport {
    VisualBenchmarkZoneKind kind{VisualBenchmarkZoneKind::NaturalForeground};
    float nominalDistance{};
    int materialVariety{};
    int structuralTriangles{};
    int decorativeTriangles{};
    int detailInstances{};
    int drawCalls{};
    int shadowCasters{};
    int particles{};
    int effectLights{};
    int effectWorkUnits{};
};

struct VisualBenchmarkReport {
    std::array<VisualBenchmarkZoneReport,static_cast<std::size_t>(VisualBenchmarkZoneKind::Count)> zones{};
    PresentationTelemetry telemetry{};
    int totalMaterialVariety{};
    int totalStructuralTriangles{};
    int totalDecorativeTriangles{};
    int totalTriangles{};
    int totalDrawCalls{};
    int totalDetailInstances{};
    int totalShadowCasters{};
    int totalParticles{};
    int totalEffectLights{};
    int totalEffectWorkUnits{};
    std::uint64_t fingerprint{};
};

struct VisualBenchmarkSettings {
    // Multiplies all zone distances. 1.0 is the documented benchmark camera;
    // larger values exercise predictable decorative LOD degradation.
    float observerDistanceScale{1.0f};
    float detailDensity{1.0f};
    EffectBudget effects{};
};

VisualBenchmarkReport buildVisualBenchmarkScene(std::uint64_t seed,
                                                const VisualBenchmarkSettings& settings = {});
const char* visualBenchmarkZoneName(VisualBenchmarkZoneKind zone);

} // namespace elysium

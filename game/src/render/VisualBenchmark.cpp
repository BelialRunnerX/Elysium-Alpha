// Intended function: imported render implementation for VisualBenchmark; preserves the agent-authored subsystem contract for later integration/debugging.
#include "render/VisualBenchmark.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <stdexcept>

namespace elysium {
namespace {

constexpr std::array<float,static_cast<std::size_t>(VisualBenchmarkZoneKind::Count)> kZoneDistances{{
    12.0f, 34.0f, 58.0f, 42.0f, 72.0f, 16.0f, 450.0f
}};

std::uint64_t mixFingerprint(std::uint64_t h, std::uint64_t value) {
    h^=value;
    h*=1099511628211ULL;
    return h;
}

void addCluster(VisualBenchmarkZoneReport& zone,
                PresentationTelemetry& telemetry,
                DetailClusterKind kind,
                std::uint64_t seed,
                float distance,
                float density,
                int materialContribution=1) {
    DetailClusterSpec spec{};
    spec.kind=kind;
    spec.seed=seed;
    spec.radius=1.0f;
    spec.density=density;
    spec.observerDistance=distance;
    const auto packet=generateDetailCluster(spec);
    accumulate(telemetry,packet);
    zone.decorativeTriangles+=packet.estimatedTriangles;
    zone.detailInstances+=static_cast<int>(packet.instances.size());
    zone.shadowCasters+=packet.shadowCasters;
    if(!packet.instances.empty()) zone.materialVariety+=materialContribution;
}

void addEffect(VisualBenchmarkZoneReport& zone,
               PresentationTelemetry& telemetry,
               PresentationEffectKind kind,
               std::uint64_t seed,
               float distance,
               float intensity,
               const EffectBudget& budget) {
    const auto packet=generateEffectPacket(kind,seed,{},intensity,distance,budget);
    accumulate(telemetry,packet);
    zone.particles+=static_cast<int>(packet.particles.size());
    zone.effectLights+=packet.dynamicLights;
    zone.effectWorkUnits+=packet.estimatedWorkUnits;
}

int creatureCost(CreatureBodyPlan plan, std::uint64_t seed, float distance) {
    if(distance>=300.0f) return 0;
    const auto rig=generateCreatureRig({plan,seed,1.0f,0.12f});
    if(distance<80.0f) return rig.estimatedTriangles;
    return std::max(24,rig.estimatedTriangles/4);
}

} // namespace

VisualBenchmarkReport buildVisualBenchmarkScene(std::uint64_t seed, const VisualBenchmarkSettings& settings) {
    if(!std::isfinite(settings.observerDistanceScale) || settings.observerDistanceScale<=0.0f)
        throw std::invalid_argument("benchmark distance scale must be positive and finite");
    if(!std::isfinite(settings.detailDensity) || settings.detailDensity<0.0f)
        throw std::invalid_argument("benchmark detail density must be finite and non-negative");

    VisualBenchmarkReport report{};
    for(std::size_t i=0;i<report.zones.size();++i) {
        report.zones[i].kind=static_cast<VisualBenchmarkZoneKind>(i);
        report.zones[i].nominalDistance=kZoneDistances[i]*settings.observerDistanceScale;
    }

    auto zoneSeed=[&](VisualBenchmarkZoneKind zone,int lane) {
        return mix64(seed ^ (static_cast<std::uint64_t>(zone)+1ULL)*0x9E3779B97F4A7C15ULL ^ static_cast<std::uint64_t>(lane));
    };

    // Natural foreground: layered terrain detail and vegetation.
    {
        auto& z=report.zones[static_cast<std::size_t>(VisualBenchmarkZoneKind::NaturalForeground)];
        z.structuralTriangles=3200;
        z.materialVariety=4;
        z.drawCalls=4;
        addCluster(z,report.telemetry,DetailClusterKind::GrassTuft,zoneSeed(z.kind,1),z.nominalDistance,settings.detailDensity);
        addCluster(z,report.telemetry,DetailClusterKind::RootBundle,zoneSeed(z.kind,2),z.nominalDistance,settings.detailDensity);
        addCluster(z,report.telemetry,DetailClusterKind::LooseRubble,zoneSeed(z.kind,3),z.nominalDistance,settings.detailDensity);
        addCluster(z,report.telemetry,DetailClusterKind::PebbleScatter,zoneSeed(z.kind,4),z.nominalDistance,settings.detailDensity);
    }

    // Frontier base: repair plates, exposed fasteners, pipes and improvised detail.
    {
        auto& z=report.zones[static_cast<std::size_t>(VisualBenchmarkZoneKind::FrontierBase)];
        z.structuralTriangles=7600;
        z.materialVariety=6;
        z.drawCalls=6;
        addCluster(z,report.telemetry,DetailClusterKind::FrontierPatch,zoneSeed(z.kind,1),z.nominalDistance,settings.detailDensity);
        addCluster(z,report.telemetry,DetailClusterKind::PipeValveCluster,zoneSeed(z.kind,2),z.nominalDistance,settings.detailDensity);
        addCluster(z,report.telemetry,DetailClusterKind::CableLoom,zoneSeed(z.kind,3),z.nominalDistance,settings.detailDensity);
        addCluster(z,report.telemetry,DetailClusterKind::HangingChain,zoneSeed(z.kind,4),z.nominalDistance,settings.detailDensity*0.6f);
    }

    // Imperial structure: black/emerald, symmetrical plated machinery and greebles.
    {
        auto& z=report.zones[static_cast<std::size_t>(VisualBenchmarkZoneKind::ImperialStructure)];
        z.structuralTriangles=10800;
        z.materialVariety=5;
        z.drawCalls=5;
        addCluster(z,report.telemetry,DetailClusterKind::PanelGreeble,zoneSeed(z.kind,1),z.nominalDistance,settings.detailDensity*1.2f);
        addCluster(z,report.telemetry,DetailClusterKind::CableLoom,zoneSeed(z.kind,2),z.nominalDistance,settings.detailDensity*0.7f);
    }

    // Industry: moving machinery proxies and utility articulation.
    {
        auto& z=report.zones[static_cast<std::size_t>(VisualBenchmarkZoneKind::Industry)];
        z.structuralTriangles=8600;
        z.materialVariety=7;
        z.drawCalls=8;
        addCluster(z,report.telemetry,DetailClusterKind::PipeValveCluster,zoneSeed(z.kind,1),z.nominalDistance,settings.detailDensity*1.4f);
        addCluster(z,report.telemetry,DetailClusterKind::CableLoom,zoneSeed(z.kind,2),z.nominalDistance,settings.detailDensity*1.2f);
        addCluster(z,report.telemetry,DetailClusterKind::ScorchDetail,zoneSeed(z.kind,3),z.nominalDistance,settings.detailDensity*0.8f);
        addEffect(z,report.telemetry,PresentationEffectKind::MetalSparks,zoneSeed(z.kind,4),z.nominalDistance,0.7f,settings.effects);
    }

    // Fauna: two body plans for entity render/animation budget.
    {
        auto& z=report.zones[static_cast<std::size_t>(VisualBenchmarkZoneKind::Fauna)];
        z.structuralTriangles=creatureCost(CreatureBodyPlan::Quadruped,zoneSeed(z.kind,1),z.nominalDistance)
                            +creatureCost(CreatureBodyPlan::Flyer,zoneSeed(z.kind,2),z.nominalDistance);
        z.materialVariety=z.structuralTriangles>0?3:0;
        z.drawCalls=z.structuralTriangles>0?2:0;
        z.shadowCasters=z.structuralTriangles>0?2:0;
    }

    // Destruction target: layered damage, debris and effects.
    {
        auto& z=report.zones[static_cast<std::size_t>(VisualBenchmarkZoneKind::DestructionTarget)];
        z.structuralTriangles=4400;
        z.materialVariety=5;
        z.drawCalls=5;
        addCluster(z,report.telemetry,DetailClusterKind::LooseRubble,zoneSeed(z.kind,1),z.nominalDistance,settings.detailDensity*1.4f);
        addCluster(z,report.telemetry,DetailClusterKind::ScorchDetail,zoneSeed(z.kind,2),z.nominalDistance,settings.detailDensity);
        addEffect(z,report.telemetry,PresentationEffectKind::StoneDebris,zoneSeed(z.kind,3),z.nominalDistance,1.0f,settings.effects);
        addEffect(z,report.telemetry,PresentationEffectKind::Smoke,zoneSeed(z.kind,4),z.nominalDistance,0.8f,settings.effects);
    }

    // Long vista intentionally carries strategic meaning without decorative detail.
    {
        auto& z=report.zones[static_cast<std::size_t>(VisualBenchmarkZoneKind::LongVista)];
        z.structuralTriangles=12500;
        z.materialVariety=4;
        z.drawCalls=4;
        // Major smoke/fire can remain visible even when small debris is gone.
        addEffect(z,report.telemetry,PresentationEffectKind::Smoke,zoneSeed(z.kind,1),z.nominalDistance,1.0f,settings.effects);
    }

    std::uint64_t fp=1469598103934665603ULL;
    for(const auto& z:report.zones) {
        report.totalMaterialVariety+=z.materialVariety;
        report.totalStructuralTriangles+=z.structuralTriangles;
        report.totalDecorativeTriangles+=z.decorativeTriangles;
        report.totalDrawCalls+=z.drawCalls;
        report.totalDetailInstances+=z.detailInstances;
        report.totalShadowCasters+=z.shadowCasters;
        report.totalParticles+=z.particles;
        report.totalEffectLights+=z.effectLights;
        report.totalEffectWorkUnits+=z.effectWorkUnits;
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(z.kind));
        fp=mixFingerprint(fp,std::bit_cast<std::uint32_t>(z.nominalDistance));
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(z.structuralTriangles));
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(z.decorativeTriangles));
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(z.detailInstances));
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(z.particles));
    }
    report.totalTriangles=report.totalStructuralTriangles+report.totalDecorativeTriangles;
    report.fingerprint=fp;
    return report;
}

const char* visualBenchmarkZoneName(VisualBenchmarkZoneKind zone) {
    static constexpr std::array<const char*,static_cast<std::size_t>(VisualBenchmarkZoneKind::Count)> names{{
        "natural_foreground","frontier_base","imperial_structure","industry","fauna","destruction_target","long_vista"
    }};
    const auto i=static_cast<std::size_t>(zone);
    return i<names.size()?names[i]:"invalid";
}

} // namespace elysium

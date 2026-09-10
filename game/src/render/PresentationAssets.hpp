// Intended function: imported render implementation for PresentationAssets; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "core/Math.hpp"
#include "world/BaseInfrastructure.hpp"
#include "world/Block.hpp"

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace elysium {

// Backend-neutral presentation data. Nothing in this file is save truth: the
// deterministic generators can be rerun from content/profile IDs + stable seeds.
// Player-authored microvoxel edits remain in the world/chunk persistence layer.

enum class MaterialFieldProfile : std::uint8_t {
    Stone = 0,
    Metal,
    ClothFiber,
    SmoothCeramic,
    Strand,
    Corroded,
    IrradiatedExotic,
    Count
};

struct MaterialFieldSpec {
    MaterialFieldProfile profile{MaterialFieldProfile::Stone};
    Color4u baseColor{128,128,128,255};
    std::uint64_t seed{};
    int width{32};
    int height{32};
    std::uint8_t variation{36};
};

struct MaterialFieldMetrics {
    float meanLuminance{};
    int luminanceRange{};
    float horizontalNeighborDelta{};
    float verticalNeighborDelta{};
    float meanRoughness{};
    int emissivePixels{};
};

struct GeneratedMaterialField {
    int width{};
    int height{};
    std::vector<Color4u> albedo;
    std::vector<std::uint8_t> roughness;
    std::vector<std::uint8_t> emissive;
    MaterialFieldMetrics metrics{};
    std::uint64_t fingerprint{};
};

GeneratedMaterialField generateMaterialField(const MaterialFieldSpec& spec);
const char* materialFieldProfileName(MaterialFieldProfile profile);

enum class FactionVisualStyle : std::uint8_t { Neutral = 0, Imperial, UnswornFrontier };

struct VisualLanguageProfile {
    FactionVisualStyle style{FactionVisualStyle::Neutral};
    Color4u primary{};
    Color4u secondary{};
    Color4u accent{};
    bool prefersBilateralSymmetry{};
    float repairAsymmetry{};
    float exposedFasteners{};
    float emissiveShare{};
};

const VisualLanguageProfile& visualLanguage(FactionVisualStyle style);

enum class DetailClusterKind : std::uint8_t {
    GrassTuft = 0,
    FlowerCluster,
    Reed,
    RootBundle,
    LooseRubble,
    PebbleScatter,
    CrystalSpray,
    IceShard,
    FungalFan,
    CoralCluster,
    CableLoom,
    PipeValveCluster,
    PanelGreeble,
    FrontierPatch,
    DustDrift,
    SnowLip,
    ScorchDetail,
    HangingChain,
    Count
};

enum class DetailLod : std::uint8_t { Full = 0, Reduced, Silhouette, Culled };
enum class DetailPrimitive : std::uint8_t { Box = 0, Blade, Prism, Segment, Plate, Link, Decal };

struct DetailInstance {
    DetailPrimitive primitive{DetailPrimitive::Box};
    Vec3 position{};
    Vec3 scale{1,1,1};
    Vec3 rotation{};
    Color4u color{255,255,255,255};
    bool castsShadow{};
};

struct DetailClusterSpec {
    DetailClusterKind kind{DetailClusterKind::GrassTuft};
    std::uint64_t seed{};
    Vec3 origin{};
    float radius{1.0f};
    float density{1.0f};
    float observerDistance{};
};

struct DetailClusterPacket {
    DetailClusterKind kind{DetailClusterKind::GrassTuft};
    DetailLod lod{DetailLod::Culled};
    std::vector<DetailInstance> instances;
    int estimatedTriangles{};
    int shadowCasters{};
    int materialVariants{};
    bool regenerable{true};
    std::uint64_t fingerprint{};
};

DetailLod detailLodForDistance(DetailClusterKind kind, float distance);
DetailClusterPacket generateDetailCluster(const DetailClusterSpec& spec);
const char* detailClusterKindName(DetailClusterKind kind);

// Generic machine/prop/structure hooks. Simulation state is read into these
// narrow inputs; presentation never becomes authoritative state.
enum class VisualActivityState : std::uint8_t { Idle = 0, Powered, Active, Fault, Damaged };

struct MachineVisualDefinition {
    std::string_view archetype;
    FactionVisualStyle style{FactionVisualStyle::Neutral};
    int baseTriangles{};
    bool articulated{};
    bool emissiveWhenPowered{};
};

struct VisualStatePacket {
    std::uint64_t stableSourceId{};
    std::uint64_t archetypeKey{};
    FactionVisualStyle style{FactionVisualStyle::Neutral};
    VisualActivityState state{VisualActivityState::Idle};
    float animationPhase{};
    float wear{};
    float emissive{};
    int estimatedTriangles{};
    Color4u accent{};
};

const MachineVisualDefinition& machineVisualDefinition(MachineType type);
VisualStatePacket buildMachineVisualState(std::uint64_t stableId,
                                          MachineType type,
                                          bool enabled,
                                          bool powered,
                                          bool active,
                                          bool faulted,
                                          float damage01,
                                          float simulationSeconds);
VisualStatePacket buildPropVisualState(std::uint64_t stableId,
                                       std::uint64_t visualArchetype,
                                       FactionVisualStyle style,
                                       bool active,
                                       float condition01,
                                       float simulationSeconds);
VisualStatePacket buildStructureVisualState(std::uint64_t stableId,
                                            std::uint64_t visualArchetype,
                                            FactionVisualStyle style,
                                            float damage01,
                                            bool powered,
                                            float simulationSeconds);

enum class CreatureBodyPlan : std::uint8_t { Quadruped = 0, Biped, Hexapod, Serpentine, Flyer, Floater, Count };
enum class RigPartRole : std::uint8_t { Root = 0, Torso, Head, Limb, Tail, Wing, Fin, Armor, Sensor };

struct RigPart {
    RigPartRole role{RigPartRole::Torso};
    int parent{-1};
    Vec3 localPosition{};
    Vec3 halfExtents{0.5f,0.5f,0.5f};
    Vec3 rotation{};
};

struct CreatureRigSpec {
    CreatureBodyPlan bodyPlan{CreatureBodyPlan::Quadruped};
    std::uint64_t seed{};
    float scale{1.0f};
    float silhouetteVariation{0.15f};
};

struct CreatureRigPacket {
    CreatureBodyPlan bodyPlan{CreatureBodyPlan::Quadruped};
    std::vector<RigPart> parts;
    int estimatedTriangles{};
    std::uint64_t fingerprint{};
};

CreatureRigPacket generateCreatureRig(const CreatureRigSpec& spec);
const char* creatureBodyPlanName(CreatureBodyPlan plan);

enum class DamageVisualKind : std::uint8_t { Crack = 0, Dent, Soot, ExposedLayer, Scorch, Frost, Corrosion };

struct DamageVisualMark {
    DamageVisualKind kind{DamageVisualKind::Crack};
    Vec3 offset{};
    Vec3 scale{1,1,1};
    float intensity{};
};

struct DamageVisualPacket {
    std::vector<DamageVisualMark> marks;
    int exposedLayers{};
    std::uint64_t fingerprint{};
};

DamageVisualPacket generateDamageVisuals(std::uint64_t stableSourceId,
                                         float damage01,
                                         bool thermal,
                                         bool corrosive,
                                         int maxMarks = 12);

enum class PresentationEffectKind : std::uint8_t {
    DustImpact = 0,
    StoneDebris,
    MetalSparks,
    Smoke,
    Fire,
    DecompressionMist,
    AcidSpray,
    RadiationPulse,
    PlasmaArc,
    ShieldHit,
    WeatherDust,
    WeatherSnow,
    WeatherSpore,
    RegisterActionSignal,
    Count
};

struct EffectBudget {
    int maxParticles{256};
    int maxInstances{64};
    int maxLights{4};
};

struct EffectParticle {
    Vec3 position{};
    Vec3 velocity{};
    float lifetime{};
    float size{};
    Color4u color{};
};

struct EffectPacket {
    PresentationEffectKind kind{PresentationEffectKind::DustImpact};
    std::vector<EffectParticle> particles;
    int instances{};
    int dynamicLights{};
    int estimatedWorkUnits{};
    std::uint64_t fingerprint{};
};

EffectPacket generateEffectPacket(PresentationEffectKind kind,
                                  std::uint64_t seed,
                                  Vec3 origin,
                                  float intensity,
                                  float observerDistance,
                                  const EffectBudget& budget = {});
const char* presentationEffectName(PresentationEffectKind kind);

enum class MaterialAudioAction : std::uint8_t { Mine = 0, Impact, Footstep, Destroy };
enum class MaterialAudioFamily : std::uint8_t { Earth = 0, Stone, Wood, Metal, GlassCeramic, Fluid, Machine, Organic };
enum class HazardAudioKind : std::uint8_t { Thermal = 0, Cryogenic, Corrosive, Radiological, Pressure, Vacuum };
enum class EmpireAudioEvent : std::uint8_t { PatrolRelay = 0, Inspection, RegisterAnnouncement, WaveStart, CourtArrival };
enum class AudioTimbre : std::uint8_t {
    EarthThud = 0,
    StoneKnock,
    WoodClack,
    MetalRing,
    GlassChime,
    FluidSplash,
    MachineHum,
    OrganicRustle,
    Wind,
    AcidHiss,
    RadiationTicks,
    PressureCreak,
    VacuumMuffle,
    OrderlyRelay,
    PrecisionServo,
    FrontierRattle
};

struct AudioCuePacket {
    AudioTimbre timbre{AudioTimbre::StoneKnock};
    float pitchHz{};
    float gain{};
    float lowFrequencyShare{};
    float pulseHz{};
    bool loop{};
    int variation{};
    std::uint64_t fingerprint{};
};

// Original procedural mono PCM. This is an optional presentation asset format,
// not authoritative simulation state, and intentionally contains no copied samples.
struct GeneratedAudioClip {
    int sampleRate{};
    int channels{1};
    bool loop{};
    std::vector<std::int16_t> pcm;
    std::uint64_t fingerprint{};
};

MaterialAudioFamily materialAudioFamily(BlockType material);
AudioCuePacket materialAudioCue(BlockType material, MaterialAudioAction action, std::uint64_t seed);
AudioCuePacket hazardAudioCue(HazardAudioKind hazard, float intensity, std::uint64_t seed);
AudioCuePacket empireAudioCue(EmpireAudioEvent event, float intensity, std::uint64_t seed);
AudioCuePacket frontierAudioCue(float intensity, std::uint64_t seed);
GeneratedAudioClip synthesizeAudioCue(const AudioCuePacket& cue, float seconds = 0.35f, int sampleRate = 22050);

struct PresentationTelemetry {
    int materialFields{};
    int materialPixels{};
    int detailClusters{};
    int detailInstances{};
    int decorativeTriangles{};
    int shadowCasters{};
    int effectPackets{};
    int particles{};
    int effectLights{};
    int effectWorkUnits{};
};

void accumulate(PresentationTelemetry& telemetry, const GeneratedMaterialField& field);
void accumulate(PresentationTelemetry& telemetry, const DetailClusterPacket& cluster);
void accumulate(PresentationTelemetry& telemetry, const EffectPacket& effect);

} // namespace elysium

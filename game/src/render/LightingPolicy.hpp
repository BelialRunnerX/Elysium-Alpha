#pragma once
#include <cstdint>
#include <vector>
namespace elysium::render {
enum class LightingQuality:std::uint8_t{Low,Medium,High,Experimental};
struct DirectionalLightPacket{double dx{},dy{},dz{},sunIntensity{},skyIntensity{};};
struct LocalLightPacket{std::uint64_t stableId{},materialId{};double x{},y{},z{},radius{},intensity{};bool emissive{},gameplayCritical{},castsShadow{};};
struct FogPacket{double density{},hazardDensity{},visibilityFloor{};std::uint64_t presentationTag{};};
struct ShadowBudget{std::uint64_t maxCasters{},maxShadowPixels{};double maxDistance{};};
struct LightingSnapshot{DirectionalLightPacket directional;std::vector<LocalLightPacket>local;FogPacket fog;std::uint64_t weatherRevision{},hazardRevision{},materialRevision{};};
struct LightingFramePacket{DirectionalLightPacket directional;std::vector<LocalLightPacket>lights,shadowCasters;FogPacket fog;LightingQuality quality{};bool indirectLightExperiment{};};
struct LightingPolicyConfig{LightingQuality quality{LightingQuality::Medium};ShadowBudget shadows{};bool enableIndirectLightExperiment{};bool profilingGatePassed{};};
class LightingPolicy{public:[[nodiscard]]LightingFramePacket build(const LightingSnapshot&,const LightingPolicyConfig&,std::uint32_t geometryLodTier)const;};
} // namespace elysium::render

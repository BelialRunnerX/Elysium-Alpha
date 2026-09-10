#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
namespace elysium::world {
enum class EnvironmentalProjectKind:std::uint8_t{LocalAtmosphere,DrainagePump,ThermalControl,CorrosionTreatment,RadiationBunker,StormShutter,TerraformingCampaignPlaceholder};
struct EnvironmentalProjectRequest{std::uint64_t projectId{},ownerId{},targetVolumeId{},powerNetworkId{},providerId{};EnvironmentalProjectKind kind{EnvironmentalProjectKind::LocalAtmosphere};std::uint64_t affectedCellEstimate{};bool providerConfirmedBounded{};bool targetsWholePlanet{};double requestedStrength{};};
struct EnvironmentalProjectPlan{bool accepted{};reason::ReasonStack reasons;double boundedStrength{};bool requiresCampaignSystem{};};
class EnvironmentalProjectPolicy{public:[[nodiscard]]EnvironmentalProjectPlan evaluate(const EnvironmentalProjectRequest&r)const;};
} // namespace elysium::world

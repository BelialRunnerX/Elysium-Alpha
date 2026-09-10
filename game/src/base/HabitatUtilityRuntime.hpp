#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <span>
#include <vector>
namespace elysium::base {
enum class HabitatUtilityKind:std::uint8_t{AtmosphereUnit,Scrubber,Heater,Cooler,WaterRecycler,AirlockController};
struct HabitatUtility{std::uint64_t stableId{},roomVolumeId{};HabitatUtilityKind kind{HabitatUtilityKind::AtmosphereUnit};double powerDemand{},rate{};bool enabled{true};};
struct HabitatInput{double pressure{},oxygen{},toxins{},smoke{},temperature{},waterAvailable{},powerAvailable{};bool sealed{};};
struct HabitatDelta{double pressure{},oxygen{},toxins{},smoke{},temperature{},water{};double powerConsumed{};};
class HabitatUtilityRuntime{public:[[nodiscard]]HabitatDelta evaluate(std::span<const HabitatUtility>utilities,const HabitatInput&input,double dt)const;};
} // namespace elysium::base

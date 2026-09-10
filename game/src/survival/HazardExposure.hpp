#pragma once
#include "survival/Survival.hpp"
#include <array>
#include <cstdint>
#include <span>
#include <vector>
namespace elysium::survival {
struct HazardSourceSample{std::uint64_t providerId{},sourceId{};SurvivalHazard hazard{SurvivalHazard::Thermal};double intensityPerSecond{};bool blockedByLocalEngineering{};};
struct ResistanceSource{std::uint64_t providerId{},sourceId{};SurvivalHazard hazard{SurvivalHazard::Thermal};double share01{};};
struct HazardContribution{std::uint64_t providerId{},sourceId{};double raw{},mitigated{};};
struct HazardExposureResult{std::array<double,kSurvivalHazardCount>raw{};std::array<double,kSurvivalHazardCount>resistance{};std::array<double,kSurvivalHazardCount>effective{};std::vector<HazardContribution>contributions;};
class HazardExposureAggregator{public:[[nodiscard]]HazardExposureResult evaluate(std::span<const HazardSourceSample>sources,std::span<const ResistanceSource>resistances)const;};
} // namespace elysium::survival

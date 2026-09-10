#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <map>
#include <span>
#include <vector>
namespace elysium::orbital {

enum class CargoClassification:std::uint8_t{Registered,Unregistered,Contraband,QuarantineRestricted};
struct ManifestLine{
 std::uint64_t contentId{},quantity{};bool registered{},contraband{},quarantineRestricted{};
 double exposureModifier{1.0};
};
struct CustomsContext{
 std::uint64_t inspectionId{},jurisdictionId{},shipId{};double suspicion{},tariffScale{1};bool quarantine{};
 std::uint64_t warrantId{},clearanceId{},inspectionSourceId{};double exposureModifier{1.0};
};
struct CargoClassificationLine{std::uint64_t contentId{},quantity{};CargoClassification classification{CargoClassification::Registered};double exposedQuantity{};};
struct CustomsDecision{
 bool cleared{};bool hold{};bool requestInterdiction{};double tariffUnits{};reason::ReasonStack reasons;
 std::uint64_t inspectionId{},warrantId{},clearanceId{};double detectedContrabandExposure{};std::vector<CargoClassificationLine> classified;
};
class CustomsInspectionPolicy{public:[[nodiscard]]CustomsDecision inspect(const CustomsContext&,std::span<const ManifestLine>)const;};

enum class InterdictionState:std::uint8_t{Active,Expired,Cancelled};
struct InterdictionField{
 std::uint64_t fieldId{},systemId{},jurisdictionId{},sourceInspectionId{},startTick{},expiresTick{},revision{1};
 double routeCostMultiplier{1.0},routeRiskAdd{},accessFriction{};InterdictionState state{InterdictionState::Active};
};
struct InterdictionEstimate{double routeCostMultiplier{1.0},routeRiskAdd{},accessFriction{};std::uint32_t activeFields{};bool alternateRoutePreserved{true};};
struct InterdictionSnapshot{std::vector<InterdictionField> fields;};
class InterdictionRegistry{
public:
 bool create(InterdictionField);
 bool cancel(std::uint64_t fieldId,std::uint64_t expectedRevision);
 void expire(std::uint64_t tick);
 [[nodiscard]]const InterdictionField*find(std::uint64_t id)const;
 [[nodiscard]]InterdictionEstimate estimate(std::uint64_t systemId,std::uint64_t tick)const;
 [[nodiscard]]InterdictionSnapshot snapshot()const;
 bool restore(const InterdictionSnapshot&);
private:std::map<std::uint64_t,InterdictionField>fields_;
};
} // namespace elysium::orbital

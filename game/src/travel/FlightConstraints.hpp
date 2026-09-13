#pragma once
#include "core/Math.hpp"
#include "world/CubeSphere.hpp"
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::travel {

enum class FlightMode : std::uint8_t { Landed, Atmospheric, Entry, Orbital };
enum class FlightBlockReason : std::uint8_t {
 None, MissingFlight, InvalidInput, IllegalTransition, MissingAtmosphericThruster,
 CargoOverLimit, StormOverLimit, ThermalEntryUnsafe, RadiationEntryUnsafe,
 InsufficientHandling
};

struct FlightConstraintLimits {
 double maximumCargoMassKg{};
 double maximumWeatherSeverity{};
 double maximumResidualThermalLoad{};
 double maximumResidualRadiationLoad{};
 double gravityHandlingWeight{};
 double cargoHandlingWeight{};
 double weatherHandlingWeight{};
 double entryHandlingWeight{};
};
struct FlightCapabilities {
 bool atmosphericThruster{};
 bool vectorControl{};
 double handlingRating{};
 double thermalMitigation{};
 double radiationMitigation{};
};
struct FlightEnvironment {
 double gravityMps2{};
 double cargoMassKg{};
 double weatherSeverity{};
 double thermalEntryLoad{};
 double radiationEntryLoad{};
 elysium::Vec3 planetDirection{0,0,1};
};
struct FlightConstraintSnapshot {
 bool valid{};
 elysium::FaceUv owner{};
 double residualThermalLoad{};
 double residualRadiationLoad{};
 double requiredHandling{};
 double availableHandling{};
 std::vector<FlightBlockReason> blockers;
};
struct FlightStateRecord {
 std::uint64_t stableShipId{};
 std::uint64_t revision{1};
 FlightMode mode{FlightMode::Landed};
 elysium::Vec3 planetDirection{0,0,1};
};
struct FlightTransition {
 bool allowed{};
 FlightBlockReason reason{FlightBlockReason::None};
 FlightMode from{FlightMode::Landed};
 FlightMode requested{FlightMode::Landed};
 FlightMode suggestedFallback{FlightMode::Landed};
 FlightConstraintSnapshot constraints;
};
struct FlightConstraintRuntimeSnapshot { std::vector<FlightStateRecord> states; };

class FlightConstraintRuntime {
public:
 bool create(FlightStateRecord);
 [[nodiscard]] const FlightStateRecord* find(std::uint64_t stableShipId) const;
 [[nodiscard]] FlightConstraintSnapshot evaluate(const FlightEnvironment&,const FlightCapabilities&,const FlightConstraintLimits&,bool entryPhase) const;
 [[nodiscard]] FlightTransition checkTransition(std::uint64_t stableShipId,FlightMode target,const FlightEnvironment&,const FlightCapabilities&,const FlightConstraintLimits&) const;
 bool commitTransition(std::uint64_t stableShipId,std::uint64_t expectedRevision,const FlightTransition&);
 [[nodiscard]] FlightTransition abortToOrbit(std::uint64_t stableShipId,const FlightEnvironment&,const FlightCapabilities&,const FlightConstraintLimits&) const;
 [[nodiscard]] FlightConstraintRuntimeSnapshot snapshot() const;
 bool restore(const FlightConstraintRuntimeSnapshot&);
private:
 std::map<std::uint64_t,FlightStateRecord> states_;
};
} // namespace elysium::travel

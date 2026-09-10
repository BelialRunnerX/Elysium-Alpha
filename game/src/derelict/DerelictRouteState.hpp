#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::derelict {
enum class RouteBlocker:std::uint8_t{None,Unpowered,Vacuum,Leak,NoGravity,SecurityLocked,Fire,Contamination,Destroyed};
enum class CounterVerb:std::uint8_t{None,RestorePower,Seal,UseZeroGMobility,BypassSecurity,Extinguish,Decontaminate,Repair};
struct SectionEnvironmentSnapshot{std::uint64_t sectionStableId{},revision{};double powerFraction{},pressureFraction{},gravityFraction{},fireLevel{},contaminationLevel{};bool leaking{},securityAwake{},securityLocked{},destroyed{};};
struct RouteCapability{bool vacuumProtection{},zeroGMobility{},securityAccess{},fireProtection{},contaminationProtection{};};
struct RouteReason{RouteBlocker blocker{RouteBlocker::None};CounterVerb counter{CounterVerb::None};double severity{};};
struct RouteState{std::uint64_t sectionStableId{},sourceRevision{};bool passable{};std::vector<RouteReason>reasons;};
struct PhysicalPersistenceIntent{std::uint64_t sectionStableId{};CounterVerb verb{};};
struct SecurityActorRequest{std::uint64_t sectionStableId{};bool wake{};};
class DerelictRouteAdapter{public:[[nodiscard]]RouteState evaluate(const SectionEnvironmentSnapshot&,const RouteCapability&)const;[[nodiscard]]std::vector<PhysicalPersistenceIntent>counterIntents(const RouteState&)const;[[nodiscard]]std::vector<SecurityActorRequest>securityRequests(const SectionEnvironmentSnapshot&)const;};
} // namespace elysium::derelict

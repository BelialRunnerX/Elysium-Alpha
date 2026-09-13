#pragma once
#include <cstdint>
#include <map>
#include <vector>

namespace elysium::orbital {

enum class OrbitalCapability : std::uint32_t { CargoDepot=1u<<0, SurveyRelay=1u<<1, Drydock=1u<<2 };
constexpr std::uint32_t orbitalCapability(OrbitalCapability c) noexcept { return static_cast<std::uint32_t>(c); }
enum class ConstructLifecycle : std::uint8_t { Construction, Active, Disabled, Destroyed };
struct OrbitalConstructDefinition {
    std::uint64_t contentId{};
    std::uint32_t capabilities{};
    std::uint64_t cargoCapacity{};
    double surveyRangeMultiplier{1.0};
    double maximumHull{};
};
struct OrbitalCargoStack { std::uint64_t itemStableId{},contentId{},count{}; };
struct OrbitalConstructState {
    std::uint64_t stableId{},definitionContentId{},systemId{},orbitAddress{},ownerStableId{},claimStableId{},revision{1};
    ConstructLifecycle lifecycle{ConstructLifecycle::Construction};
    bool powered{};
    double hull{};
    std::vector<OrbitalCargoStack> cargo;
};
enum class DepotTransferDirection:std::uint8_t{IntoDepot,OutOfDepot};
enum class ServiceTransactionState:std::uint8_t{Requested,Reserved,Committed,Cancelled};
struct DepotTransferTransaction { std::uint64_t stableId{},constructId{},externalEndpointId{},itemStableId{},contentId{},count{},revision{1};DepotTransferDirection direction{};ServiceTransactionState state{ServiceTransactionState::Requested}; };
enum class DrydockOperation:std::uint8_t{InstallModule,RepairHull};
struct DrydockTransaction { std::uint64_t stableId{},constructId{},shipStableId{},moduleItemStableId{},materialContentId{},materialCount{},revision{1};double requestedHullRepair{};DrydockOperation operation{};ServiceTransactionState state{ServiceTransactionState::Requested}; };
enum class OrbitalServiceFailure:std::uint8_t{None,MissingConstruct,UnknownDefinition,WrongCapability,Inactive,Unpowered,CapacityExceeded,MissingCargo,DuplicateTransaction,InvalidRequest};
struct SurveyRelayProjection { bool available{}; double rangeMultiplier{1.0}; std::uint64_t constructStableId{}; };
struct OrbitalConstructSnapshot { std::vector<OrbitalConstructDefinition>definitions;std::vector<OrbitalConstructState>constructs;std::vector<DepotTransferTransaction>depotTransactions;std::vector<DrydockTransaction>drydockTransactions; };
class OrbitalConstructRuntime{public:
 bool publish(OrbitalConstructDefinition); bool create(OrbitalConstructState); bool setLifecycle(std::uint64_t,ConstructLifecycle); bool setPowered(std::uint64_t,bool);
 [[nodiscard]]const OrbitalConstructState*find(std::uint64_t)const;[[nodiscard]]SurveyRelayProjection relay(std::uint64_t)const;
 OrbitalServiceFailure beginDepotTransfer(DepotTransferTransaction);OrbitalServiceFailure commitDepotTransfer(std::uint64_t);bool cancelDepotTransfer(std::uint64_t);
 OrbitalServiceFailure beginDrydock(DrydockTransaction);OrbitalServiceFailure commitDrydock(std::uint64_t);bool cancelDrydock(std::uint64_t);
 std::uint64_t insertCargo(std::uint64_t,OrbitalCargoStack);std::uint64_t extractCargo(std::uint64_t,std::uint64_t,std::uint64_t);
 [[nodiscard]]OrbitalConstructSnapshot snapshot()const;bool restore(const OrbitalConstructSnapshot&);
private:
 bool operational(const OrbitalConstructState&,OrbitalCapability)const;std::map<std::uint64_t,OrbitalConstructDefinition>definitions_;std::map<std::uint64_t,OrbitalConstructState>constructs_;std::map<std::uint64_t,DepotTransferTransaction>depotTransactions_;std::map<std::uint64_t,DrydockTransaction>drydockTransactions_;
};
} // namespace elysium::orbital

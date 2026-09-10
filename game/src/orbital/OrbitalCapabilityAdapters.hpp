#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::orbital {
enum class AdvancedOrbitalKind:std::uint8_t{Refinery,DefensePlatform,GateAnchor};
struct ResourceQuantity{std::uint64_t contentId{},count{};};
struct AdvancedOrbitalState{std::uint64_t stableId{},parentConstructId{},systemId{},ownerStableId{},claimStableId{},revision{1};AdvancedOrbitalKind kind{};bool enabled{true},powered{};double integrity{1.0},exposureMultiplier{1.0};std::uint64_t gateRequiredMask{},gateSatisfiedMask{};bool gateActive{};};
struct RefineryBatchRequest{std::uint64_t transactionId{},constructId{},recipeContentId{};std::vector<ResourceQuantity>inputs,outputs;double standingSignal{};};
struct StandingEmission{std::uint64_t sourceStableId{},systemId{},reasonContentId{};double magnitude{};};
struct DefenseProjection{bool available{};std::uint64_t constructStableId{},systemId{},claimStableId{};double readiness{};};
struct GateProjection{bool active{};std::uint64_t constructStableId{},systemId{},routeCapabilityKey{};};
enum class AdapterFailure:std::uint8_t{None,MissingConstruct,WrongKind,Disabled,Unpowered,Damaged,InvalidRequest,DuplicateTransaction,MissingInput,PrerequisiteMissing};
struct AdvancedOrbitalSnapshot{std::vector<AdvancedOrbitalState>states;std::vector<std::pair<std::uint64_t,std::vector<ResourceQuantity>>>inventories;std::vector<std::uint64_t>processedTransactions;};
class OrbitalCapabilityAdapters{public:
 bool create(AdvancedOrbitalState);bool setPowered(std::uint64_t,bool);bool setEnabled(std::uint64_t,bool);bool repair(std::uint64_t,double);bool setGateSatisfiedMask(std::uint64_t,std::uint64_t);
 std::uint64_t insertResource(std::uint64_t,ResourceQuantity);std::uint64_t resourceCount(std::uint64_t,std::uint64_t)const;
 AdapterFailure runRefineryBatch(const RefineryBatchRequest&,StandingEmission* emission=nullptr);[[nodiscard]]DefenseProjection defense(std::uint64_t)const;AdapterFailure activateGate(std::uint64_t);bool deactivateGate(std::uint64_t);[[nodiscard]]GateProjection gate(std::uint64_t)const;
 [[nodiscard]]const AdvancedOrbitalState*find(std::uint64_t)const;[[nodiscard]]AdvancedOrbitalSnapshot snapshot()const;bool restore(const AdvancedOrbitalSnapshot&);
private:std::map<std::uint64_t,AdvancedOrbitalState>states_;std::map<std::uint64_t,std::map<std::uint64_t,std::uint64_t>>inventory_;std::map<std::uint64_t,bool>processed_;
};
} // namespace elysium::orbital

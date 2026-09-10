#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::contracts {
enum class ContractKind:std::uint8_t{Survey,Procurement,Construction,Recovery,Escort,Bounty,Infrastructure};
enum class ObjectiveKind:std::uint8_t{Discover,Deliver,Build,Recover,Escort,Defeat,Connect};
enum class ContractState:std::uint8_t{Offered,Active,Completed,Failed,Cancelled};
enum class ContractFailure:std::uint8_t{None,Invalid,WrongState,DeadlineExpired,TargetMismatch,RevisionConflict,DuplicateOperation};
struct ContractObjective{std::uint64_t stableId{};ObjectiveKind kind{};std::uint64_t targetStableId{},required{},progress{};bool allowPartial{};};
struct ContractReward{std::uint64_t contentId{},count{};};
struct ContractInstance{std::uint64_t stableId{},issuerStableId{},revision{1},acceptedTick{},deadlineTick{},sourceCommissionStableId{};ContractKind kind{};ContractState state{ContractState::Offered};std::vector<ContractObjective>objectives;std::vector<ContractReward>rewards;};
struct ContractOperation{std::uint64_t stableId{},contractStableId{},expectedRevision{},tick{},objectiveStableId{},targetStableId{},amount{};enum class Kind:std::uint8_t{Accept,Progress,Complete,Fail,Cancel}kind{};};
struct ContractEvent{std::uint64_t contractStableId{};ContractState state{};bool rewardSettlement{};std::vector<ContractReward>rewards;};
struct ContractResult{bool committed{},duplicate{};ContractFailure failure{};std::uint64_t revision{};std::vector<ContractEvent>events;};
struct ContractSnapshot{std::vector<ContractInstance>contracts;std::vector<std::uint64_t>processedOperationIds;};
class ContractRuntime{public:bool publish(ContractInstance);ContractResult apply(const ContractOperation&);ContractResult expire(std::uint64_t contractId,std::uint64_t tick,std::uint64_t operationId);[[nodiscard]]const ContractInstance*find(std::uint64_t)const;[[nodiscard]]ContractSnapshot snapshot()const;bool restore(const ContractSnapshot&);private:bool objectivesComplete(const ContractInstance&)const;std::map<std::uint64_t,ContractInstance>contracts_;std::map<std::uint64_t,ContractResult>processed_;std::map<std::uint64_t,bool>settled_;};
} // namespace elysium::contracts

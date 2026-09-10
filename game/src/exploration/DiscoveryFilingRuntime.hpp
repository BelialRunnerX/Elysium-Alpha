#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>
namespace elysium::exploration {
enum class FilingDestination:std::uint8_t{Empire,Unsworn,Private};
enum class RenameAuthority:std::uint8_t{DiscovererOnly,OwnerOrDiscoverer,Institutional,Locked};
enum class FilingState:std::uint8_t{Requested,Committed,RolledBack};
enum class StandingLedger:std::uint8_t{Suspicion,Favor};
struct NamingPolicy{std::uint64_t maximumUtf8Bytes{};bool allowLeadingTrailingSpace{};};
struct DiscoveryIdentity{std::uint64_t stableTargetId{},proceduralIdentityKey{},discovererStableId{},ownerStableId{},revision{1};std::string displayName;RenameAuthority renameAuthority{RenameAuthority::DiscovererOnly};};
struct FilingRewardDefinition{FilingDestination destination{};std::uint64_t credits{};bool propagateCharts{};bool exposeExternally{};StandingLedger standingLedger{StandingLedger::Suspicion};double standingDelta{};};
struct StandingIntent{StandingLedger ledger{};double delta{};std::uint64_t sourceStableId{};};
struct FilingTransaction{std::uint64_t stableId{},discoveryStableId{},authorStableId{},revision{1};FilingDestination destination{};FilingState state{FilingState::Requested};};
struct FilingHistoryRecord{std::uint64_t transactionStableId{},discoveryStableId{},authorStableId{};FilingDestination destination{};std::uint64_t creditsGranted{};bool chartPropagationRequested{},externalExposure{};double standingDelta{};StandingLedger standingLedger{};};
enum class FilingFailure:std::uint8_t{None,MissingDiscovery,InvalidName,PermissionDenied,DuplicateTransaction,AlreadyFiled,MissingRewardProfile,InvalidTransactionState};
struct FilingOutcome{bool committed{};FilingFailure failure{FilingFailure::None};std::uint64_t creditsGrant{};bool chartPropagationRequest{},externalExposure{};StandingIntent standing{};};
struct FilingSnapshot{std::vector<DiscoveryIdentity>discoveries;std::vector<FilingRewardDefinition>rewards;std::vector<FilingTransaction>transactions;std::vector<FilingHistoryRecord>history;};
class DiscoveryFilingRuntime{public:
 bool publishReward(FilingRewardDefinition);FilingFailure discover(DiscoveryIdentity,const NamingPolicy&);FilingFailure rename(std::uint64_t targetId,std::uint64_t authorStableId,std::string newName,const NamingPolicy&,bool institutionalAuthority=false);FilingFailure begin(FilingTransaction);FilingOutcome commit(std::uint64_t transactionId);bool rollback(std::uint64_t transactionId);[[nodiscard]]const DiscoveryIdentity*find(std::uint64_t)const;[[nodiscard]]const std::vector<FilingHistoryRecord>&history()const noexcept{return history_;}[[nodiscard]]FilingSnapshot snapshot()const;bool restore(const FilingSnapshot&);
private:bool validName(const std::string&,const NamingPolicy&)const;bool canRename(const DiscoveryIdentity&,std::uint64_t,bool)const;std::map<std::uint64_t,DiscoveryIdentity>discoveries_;std::map<FilingDestination,FilingRewardDefinition>rewards_;std::map<std::uint64_t,FilingTransaction>transactions_;std::vector<FilingHistoryRecord>history_;
};
} // namespace elysium::exploration

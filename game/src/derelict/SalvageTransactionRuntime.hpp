#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::derelict {
enum class SalvageTxnState:std::uint8_t{Requested,InProgress,Paused,Completed,Cancelled};
enum class SalvageFailure:std::uint8_t{None,Invalid,AlreadySalvaged,TooHeavy,NoCargoVolume,NoTransferLink,RevisionConflict,DuplicateTransaction};
struct SalvageItem{std::uint64_t stableId{},contentId{},count{},unitMass{},unitVolume{},remainingCount{};};
struct CargoCapacitySnapshot{std::uint64_t endpointStableId{},revision{},massCapacity{},massUsed{},volumeCapacity{},volumeUsed{};bool transferLinked{};};
struct SalvageTransaction{std::uint64_t stableId{},sourceContainerId{},itemStableId{},cargoEndpointId{},expectedCargoRevision{},requestedCount{},movedCount{},revision{1};SalvageTxnState state{SalvageTxnState::Requested};};
struct SalvageResult{bool committed{},duplicate{};SalvageFailure failure{SalvageFailure::None};std::uint64_t movedCount{},remainingCount{};};
struct SalvageSnapshot{std::vector<SalvageItem>items;std::vector<SalvageTransaction>transactions;};
class SalvageRuntime{public:bool publish(SalvageItem);SalvageFailure begin(SalvageTransaction);SalvageResult progress(std::uint64_t transactionId,CargoCapacitySnapshot&);bool cancel(std::uint64_t);[[nodiscard]]const SalvageItem*findItem(std::uint64_t)const;[[nodiscard]]SalvageSnapshot snapshot()const;bool restore(const SalvageSnapshot&);private:std::map<std::uint64_t,SalvageItem>items_;std::map<std::uint64_t,SalvageTransaction>transactions_;std::map<std::uint64_t,SalvageResult>processed_;};
} // namespace elysium::derelict

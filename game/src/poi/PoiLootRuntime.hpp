#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <vector>
namespace elysium::poi {
enum class PoiObjectProvenance:std::uint8_t{GeneratedAuthored,PlayerPlaced,Unknown};
enum class PoiInteractionKind:std::uint8_t{Open,Repair,Destroy};
enum class PoiInteractionState:std::uint8_t{Requested,Committed,RolledBack};
enum class PoiLootFailure:std::uint8_t{None,UnknownPool,InvalidObject,DuplicateTransaction,AlreadyConsumed,UnapprovedReward,ProvenanceRejected,InvalidState};
enum class PoiSideEffectKind:std::uint8_t{Standing,Research,Discovery};
struct RewardEntry{std::uint64_t rewardContentId{},weight{},minimumCount{},maximumCount{};};
struct PoiLootPool{std::uint64_t contentId{};std::vector<RewardEntry>entries;};
struct PoiLootKey{std::uint64_t worldSeed{},siteStableId{},objectStableId{},poolContentId{};PoiObjectProvenance provenance{PoiObjectProvenance::GeneratedAuthored};};
struct RewardGrant{std::uint64_t rewardContentId{},count{};};
struct PoiSideEffectIntent{PoiSideEffectKind kind{};std::uint64_t sourceStableId{},contentId{};double scalar{};};
struct PoiLootTransaction{std::uint64_t stableId{},actorStableId{};PoiLootKey key{};PoiInteractionKind kind{PoiInteractionKind::Open};PoiInteractionState state{PoiInteractionState::Requested};std::uint64_t revision{1};};
struct PoiLootObjectState{std::uint64_t objectStableId{},revision{1};bool consumed{},tombstoned{};};
struct PoiLootOutcome{bool committed{};PoiLootFailure failure{PoiLootFailure::None};std::vector<RewardGrant>rewards;std::vector<PoiSideEffectIntent>effects;};
struct PoiLootSnapshot{std::vector<PoiLootPool>pools;std::vector<std::uint64_t>approvedRewardIds;std::vector<PoiLootTransaction>transactions;std::vector<PoiLootObjectState>touchedObjects;};
class PoiLootRuntime{public:bool approveReward(std::uint64_t);PoiLootFailure publish(PoiLootPool);PoiLootFailure begin(PoiLootTransaction);PoiLootOutcome commit(std::uint64_t);bool rollback(std::uint64_t);[[nodiscard]]const PoiLootObjectState*objectState(std::uint64_t)const;[[nodiscard]]PoiLootSnapshot snapshot()const;bool restore(const PoiLootSnapshot&);private:std::vector<RewardGrant>roll(const PoiLootKey&)const;std::map<std::uint64_t,PoiLootPool>pools_;std::set<std::uint64_t>approved_;std::map<std::uint64_t,PoiLootTransaction>transactions_;std::map<std::uint64_t,PoiLootObjectState>objects_;};
} // namespace elysium::poi

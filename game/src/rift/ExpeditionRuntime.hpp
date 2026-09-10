#pragma once
#include "rift/RiftTopology.hpp"
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::rift {
enum class ExpeditionState:std::uint8_t{Active,Extracted,Failed};
enum class ExpeditionTxnKind:std::uint8_t{PushDeeper,BankExtract,FailRun};
enum class ExpeditionFailure:std::uint8_t{None,InvalidSession,InvalidTransaction,DuplicateTransaction,RevisionConflict,WrongState};
enum class RiftPlanetTheme:std::uint8_t{Barren,Temperate,Scorched,Frozen,Toxic,Irradiated,Oceanic,Anomalous};
struct ExpeditionReward{std::uint64_t contentId{},count{};bool operator==(const ExpeditionReward&)const=default;};
struct ExpeditionBoon{std::uint64_t contentId{};bool permanent{};std::uint64_t acquiredDepth{};bool operator==(const ExpeditionBoon&)const=default;};
struct StandingPromotionIntent{std::uint64_t sessionStableId{},systemStableId{};double heat{};bool operator==(const StandingPromotionIntent&)const=default;};
struct ExpeditionSession{std::uint64_t stableId{},riftStableId{},systemStableId{},revision{1},currentDepth{};double heat{};ExpeditionState state{ExpeditionState::Active};std::vector<ExpeditionReward>banked,unbanked;std::vector<ExpeditionBoon>boons;bool operator==(const ExpeditionSession&)const=default;};
struct ExpeditionTransaction{std::uint64_t stableId{},sessionStableId{},expectedRevision{};ExpeditionTxnKind kind{ExpeditionTxnKind::PushDeeper};std::uint64_t depthDelta{1};double heatDelta{};std::vector<ExpeditionReward>rewardDelta;std::vector<ExpeditionBoon>boonDelta;};
struct HeatPromotionPolicy{double threshold{100.0};};
struct ExpeditionResult{bool committed{};bool duplicate{};ExpeditionFailure failure{ExpeditionFailure::None};std::uint64_t revision{};std::vector<StandingPromotionIntent>standingIntents;};
struct RiftRoomTheme{std::uint64_t roomStableId{},materialProfileId{},hazardProfileId{},encounterProfileId{};bool anomalousRule{};bool operator==(const RiftRoomTheme&)const=default;};
struct ExpeditionSnapshot{std::vector<ExpeditionSession>sessions;std::vector<std::uint64_t>processedTransactionIds;};
class ExpeditionRuntime{
public:
 bool create(ExpeditionSession);
 [[nodiscard]]const ExpeditionSession*find(std::uint64_t)const;
 ExpeditionResult commit(const ExpeditionTransaction&,const HeatPromotionPolicy& policy={});
 [[nodiscard]]std::vector<RiftRoomTheme>theme(const RiftTopology&,RiftPlanetTheme,std::uint64_t universeSeed)const;
 [[nodiscard]]ExpeditionSnapshot snapshot()const;
 bool restore(const ExpeditionSnapshot&);
private:
 std::map<std::uint64_t,ExpeditionSession>sessions_;
 std::map<std::uint64_t,ExpeditionResult>processed_;
};
} // namespace elysium::rift

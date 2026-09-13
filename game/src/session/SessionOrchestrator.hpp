#pragma once
#include "save/SpatialDeltaCodec.hpp"
#include <cstdint>
#include <functional>
#include <map>
#include <vector>
namespace elysium::session {
enum class CommandStatus:std::uint8_t{Predicted,Confirmed,Rejected};
enum class AuthorityFailure:std::uint8_t{None,InvalidIntent,SequenceConflict,ValidationRejected,PersistentWriteRejected};
struct SessionCommandIntent{std::uint64_t sequence{},actorStableId{},targetStableId{},commandContentId{},expectedRevision{};std::vector<std::uint8_t>payload;};
struct PredictionRecord{SessionCommandIntent intent;CommandStatus status{CommandStatus::Predicted};std::uint64_t authoritativeRevision{};};
struct PresentSnapshot{std::uint64_t sessionStableId{},tick{},revision{};std::vector<PredictionRecord>commands;};
struct AuthorityCommit{bool accepted{};std::uint64_t newRevision{};std::vector<elysium::save::SpatialDelta>persistentDeltas;};
struct AuthorityResult{bool accepted{};AuthorityFailure failure{};std::uint64_t sequence{},revision{};std::vector<elysium::save::SpatialDelta>publishedDeltas;};
class SessionOrchestrator{
public:
 explicit SessionOrchestrator(std::uint64_t sessionStableId):sessionStableId_(sessionStableId){}
 bool predict(SessionCommandIntent);
 AuthorityResult authorityProcess(std::uint64_t sequence,const std::function<AuthorityCommit(const SessionCommandIntent&)>& validatorCommit,elysium::save::SpatialDeltaStore& authorityStore);
 bool reject(std::uint64_t sequence,AuthorityFailure=AuthorityFailure::ValidationRejected);
 [[nodiscard]]PresentSnapshot present(std::uint64_t tick)const;
 [[nodiscard]]const PredictionRecord*prediction(std::uint64_t sequence)const;
private:std::uint64_t sessionStableId_{},revision_{1};std::map<std::uint64_t,PredictionRecord>predictions_;
};
} // namespace elysium::session

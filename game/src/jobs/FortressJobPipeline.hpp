#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::jobs {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class JobState:std::uint8_t{Queued,Reserved,Assigned,Executing,CommitPending,Completed,Failed,Cancelled};struct Job{StableId id{},sourceDesignation{},worker{},target{},reservation{};ContentId type{};std::uint16_t priority{};JobState state{JobState::Queued};std::uint64_t worldRevision{};};struct WorkerCandidate{StableId worker{};double score{};bool eligible{true};};class FortressJobPipeline{public:bool add(Job,std::string&);std::optional<StableId>assign(StableId,const std::vector<WorkerCandidate>&,std::string&);bool reserve(StableId,StableId,std::string&);bool begin(StableId,std::uint64_t currentWorldRevision,std::string&);bool markCommitPending(StableId);bool settle(StableId,bool success);std::optional<Job>get(StableId)const;private:std::unordered_map<StableId,Job>jobs_;std::unordered_map<StableId,StableId>reservations_;};}
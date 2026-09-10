#pragma once
#include "core/JobSystem.hpp"
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <vector>
namespace elysium::jobs {
enum class WorkClass:std::uint8_t{EditRemesh,VisibleGeneration,VisibleMesh,Prefetch,Lod,AiThink,AiPerception,PoiPrep,Validation,Telemetry,Compaction};
enum class WorkPriority:std::uint8_t{Critical=0,High=1,Medium=2,Low=3};
enum class JobDisposition:std::uint8_t{Queued,Dropped,Cancelled,Computed,Published,Failed};
struct VersionToken{std::uint64_t domainStableId{},version{};};
struct ScheduledJob{std::uint64_t stableId{},sequence{};WorkClass workClass{};WorkPriority priority{};VersionToken token{};bool publicationRequired{};};
struct SchedulerPolicy{std::size_t softQueueLimit{128},hardQueueLimit{512};};
struct SchedulerStats{std::uint64_t queued{},dropped{},cancelled{},computed{},published{},failed{};};
class PriorityJobScheduler{
public:
 explicit PriorityJobScheduler(SchedulerPolicy policy={}):policy_(policy){}
 bool setCurrentVersion(std::uint64_t domainStableId,std::uint64_t version);
 JobDisposition enqueue(ScheduledJob,std::function<bool()> compute,std::function<void()> publish={});
 bool pumpOne(elysium::JobSystem&);
 void drain(elysium::JobSystem&);
 [[nodiscard]]SchedulerStats stats()const;
 [[nodiscard]]std::size_t queued()const;
private:
 struct Entry{ScheduledJob job;std::function<bool()> compute;std::function<void()> publish;};
 static WorkPriority canonicalPriority(WorkClass);
 bool stale(const ScheduledJob&)const;
 bool shed(WorkClass)const;
 mutable std::mutex mutex_;SchedulerPolicy policy_;std::vector<Entry>queue_;std::map<std::uint64_t,std::uint64_t>versions_;std::uint64_t nextSequence_{1};SchedulerStats stats_{};
};
} // namespace elysium::jobs

#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <vector>
namespace elysium::perf {
enum class Metric : std::uint8_t { EditVisibleMs,ChunkGenerationMs,MeshingMs,ResidentMemoryMb,FrameMs,StreamingQueue,SaveFlushMs,GpuTriangles,GpuInstances,WorkerUtilization,EcsAiMs };
enum class Evidence : std::uint8_t { Tuning,Estimate,MeasuredHeadless,MeasuredNative };
enum class Degradation : std::uint8_t { None,DeferPrefetch,ThinDetail,ReduceShadows,LowerAnimationRate };
struct Budget { Metric metric{}; double target{}; double recoverBelow{}; Evidence evidence{Evidence::Tuning}; };
struct Sample { Metric metric{}; double value{}; std::uint64_t tick{}; Evidence evidence{Evidence::MeasuredHeadless}; };
struct Snapshot { std::map<Metric,Sample> latest; Degradation degradation{Degradation::None}; bool overBudget{}; };
class Coordinator { public: bool setBudget(Budget); bool publish(Sample); Snapshot evaluate(); const std::map<Metric,Budget>& budgets() const{return budgets_;} private: std::map<Metric,Budget> budgets_; std::map<Metric,Sample> samples_; Degradation stage_{Degradation::None}; unsigned healthyPasses_{}; };
}

// Intended function: sleep inactive factory clusters and apply bounded coarse catch-up without simulating every conveyor/machine every frame.
#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace elysium {
enum class FactoryWakeReason:std::uint8_t{None,PlayerNear,PowerChanged,InputArrived,OutputRequested,Alert,Explicit};
struct FactoryClusterState{std::uint64_t clusterId{};bool sleeping{};std::uint64_t lastFineTick{},lastCoarseTick{};std::uint32_t machineCount{};double accumulatedSeconds{};FactoryWakeReason wakeReason{FactoryWakeReason::None};};
struct FactoryCatchupSlice{std::uint64_t clusterId{};double seconds{};std::uint32_t maxProcessCompletions{};};
class FactorySleepController{public:
 void upsert(std::uint64_t clusterId,std::uint32_t machines,std::uint64_t tick);
 void noteActivity(std::uint64_t clusterId,FactoryWakeReason reason,std::uint64_t tick);
 void evaluateSleep(std::uint64_t tick,std::uint64_t idleTicks);
 std::vector<FactoryCatchupSlice> collectCatchup(double dt,std::uint32_t globalCompletionBudget);
 const std::unordered_map<std::uint64_t,FactoryClusterState>& clusters()const{return clusters_;}
private:std::unordered_map<std::uint64_t,FactoryClusterState> clusters_;
};
} // namespace elysium

#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace elysium::devtools {
using StableId=std::uint64_t; using TypeId=std::uint64_t;
struct SimulationRecord { StableId stableId{}; TypeId componentOrEvent{}; std::uint64_t stateHash{}; std::uint64_t tick{}; std::uint32_t outcome{}; };
struct GraphRecord { StableId node{}; StableId network{}; std::uint64_t supply{}; std::uint64_t demand{}; std::uint64_t throughput{}; std::uint64_t blockerCode{}; };
struct PerformanceRecord { std::uint64_t metric{}; double value{}; std::uint64_t tick{}; std::uint8_t evidenceClass{}; };
class SimulationRecorder { public: bool record(SimulationRecord); bool graph(GraphRecord); bool performance(PerformanceRecord); std::string report() const; private: std::vector<SimulationRecord> sim_; std::vector<GraphRecord> graphs_; std::vector<PerformanceRecord> perf_; };
}

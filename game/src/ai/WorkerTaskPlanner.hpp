#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Expand assigned work orders into tool, resource, movement, interaction, and completion action sequences.
struct WorkerTaskPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct WorkerTaskPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class WorkerTaskPlannerStore { public: bool apply(const WorkerTaskPlannerOp&); bool erase(std::uint64_t); const WorkerTaskPlannerData* find(std::uint64_t) const; std::vector<WorkerTaskPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,WorkerTaskPlannerData> data_; };
}

#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build work orders, priorities, assignees, reservations, blockers, queues, and cancellation/reassignment command intents.
struct JobsScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct JobsScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class JobsScreenModelStore { public: bool apply(const JobsScreenModelOp&); bool erase(std::uint64_t); const JobsScreenModelData* find(std::uint64_t) const; std::vector<JobsScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,JobsScreenModelData> data_; };
}

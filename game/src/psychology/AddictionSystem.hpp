#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track bounded substance/behavior dependencies, tolerance, withdrawal, treatment, relapse, and social/work consequences.
struct AddictionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct AddictionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class AddictionSystemStore { public: bool apply(const AddictionSystemOp&); bool erase(std::uint64_t); const AddictionSystemData* find(std::uint64_t) const; std::vector<AddictionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,AddictionSystemData> data_; };
}

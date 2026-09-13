#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Store bounded personal memories with subject, valence, importance, decay, and relationship effects.
struct MemorySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MemorySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MemorySystemStore { public: bool apply(const MemorySystemOp&); bool erase(std::uint64_t); const MemorySystemData* find(std::uint64_t) const; std::vector<MemorySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MemorySystemData> data_; };
}

#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build bounded biographies from Chronicle-linked births, relationships, work, travel, discoveries, conflicts, injuries, and achievements.
struct BiographySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BiographySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BiographySystemStore { public: bool apply(const BiographySystemOp&); bool erase(std::uint64_t); const BiographySystemData* find(std::uint64_t) const; std::vector<BiographySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BiographySystemData> data_; };
}

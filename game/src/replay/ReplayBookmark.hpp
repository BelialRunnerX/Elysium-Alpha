#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Store named tick ranges, camera hints, subjects, and diagnostic notes for debugging and content capture.
struct ReplayBookmarkOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ReplayBookmarkData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ReplayBookmarkStore { public: bool apply(const ReplayBookmarkOp&); bool erase(std::uint64_t); const ReplayBookmarkData* find(std::uint64_t) const; std::vector<ReplayBookmarkData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ReplayBookmarkData> data_; };
}

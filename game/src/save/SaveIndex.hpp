#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Maintain lightweight save metadata and stable record indexes for bounded lookup without loading entire worlds.
struct SaveIndexOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SaveIndexData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SaveIndexStore { public: bool apply(const SaveIndexOp&); bool erase(std::uint64_t); const SaveIndexData* find(std::uint64_t) const; std::vector<SaveIndexData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SaveIndexData> data_; };
}

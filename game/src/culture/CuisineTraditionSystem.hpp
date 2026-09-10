#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track cuisines, ingredients, preparation styles, dietary rules, festival foods, trade influence, and migration changes.
struct CuisineTraditionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CuisineTraditionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CuisineTraditionSystemStore { public: bool apply(const CuisineTraditionSystemOp&); bool erase(std::uint64_t); const CuisineTraditionSystemData* find(std::uint64_t) const; std::vector<CuisineTraditionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CuisineTraditionSystemData> data_; };
}

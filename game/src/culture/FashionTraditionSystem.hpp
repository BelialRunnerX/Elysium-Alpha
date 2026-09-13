#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track clothing/armor aesthetics, materials, status signals, uniforms, subcultures, and production demand.
struct FashionTraditionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FashionTraditionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FashionTraditionSystemStore { public: bool apply(const FashionTraditionSystemOp&); bool erase(std::uint64_t); const FashionTraditionSystemData* find(std::uint64_t) const; std::vector<FashionTraditionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FashionTraditionSystemData> data_; };
}

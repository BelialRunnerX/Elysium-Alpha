#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Coordinate queued actor/item/vehicle passage through pressure boundaries without owning atmosphere simulation.
struct AirlockTransitOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct AirlockTransitData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class AirlockTransitStore { public: bool apply(const AirlockTransitOp&); bool erase(std::uint64_t); const AirlockTransitData* find(std::uint64_t) const; std::vector<AirlockTransitData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,AirlockTransitData> data_; };
}

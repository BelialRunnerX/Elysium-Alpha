#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate ship construction, owners, voyages, battles, refits, accidents, crew stories, and derelict transitions.
struct ShipHistoryGeneratorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipHistoryGeneratorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipHistoryGeneratorStore { public: bool apply(const ShipHistoryGeneratorOp&); bool erase(std::uint64_t); const ShipHistoryGeneratorData* find(std::uint64_t) const; std::vector<ShipHistoryGeneratorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipHistoryGeneratorData> data_; };
}

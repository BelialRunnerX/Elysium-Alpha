#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate faction-level expansion, diplomacy, trade, military, research, espionage, and crisis goals.
struct FactionStrategistAIOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FactionStrategistAIData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FactionStrategistAIStore { public: bool apply(const FactionStrategistAIOp&); bool erase(std::uint64_t); const FactionStrategistAIData* find(std::uint64_t) const; std::vector<FactionStrategistAIData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FactionStrategistAIData> data_; };
}

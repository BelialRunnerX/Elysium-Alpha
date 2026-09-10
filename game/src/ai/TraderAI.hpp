#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate travel, buying, selling, negotiation, risk, cargo, and destination decisions for autonomous traders.
struct TraderAIOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TraderAIData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TraderAIStore { public: bool apply(const TraderAIOp&); bool erase(std::uint64_t); const TraderAIData* find(std::uint64_t) const; std::vector<TraderAIData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TraderAIData> data_; };
}

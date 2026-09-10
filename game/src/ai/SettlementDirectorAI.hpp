#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate settlement-level priorities from shortages, threats, citizen needs, construction, industry, and strategic directives.
struct SettlementDirectorAIOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SettlementDirectorAIData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SettlementDirectorAIStore { public: bool apply(const SettlementDirectorAIOp&); bool erase(std::uint64_t); const SettlementDirectorAIData* find(std::uint64_t) const; std::vector<SettlementDirectorAIData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SettlementDirectorAIData> data_; };
}

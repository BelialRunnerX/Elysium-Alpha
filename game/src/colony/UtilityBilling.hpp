#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track settlement utility usage, subsidies, rationing, fees, exemptions, and service shutoff policy hooks.
struct UtilityBillingOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct UtilityBillingData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class UtilityBillingStore { public: bool apply(const UtilityBillingOp&); bool erase(std::uint64_t); const UtilityBillingData* find(std::uint64_t) const; std::vector<UtilityBillingData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,UtilityBillingData> data_; };
}

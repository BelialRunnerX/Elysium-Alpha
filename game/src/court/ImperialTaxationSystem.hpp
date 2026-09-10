#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track Imperial dues, tariffs, tribute, exemptions, arrears, audits, enforcement, and negotiation.
struct ImperialTaxationSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ImperialTaxationSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ImperialTaxationSystemStore { public: bool apply(const ImperialTaxationSystemOp&); bool erase(std::uint64_t); const ImperialTaxationSystemData* find(std::uint64_t) const; std::vector<ImperialTaxationSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ImperialTaxationSystemData> data_; };
}

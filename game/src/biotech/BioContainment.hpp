#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track containment levels, labs, quarantine boundaries, decontamination, breaches, and emergency shutdown.
struct BioContainmentOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BioContainmentData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BioContainmentStore { public: bool apply(const BioContainmentOp&); bool erase(std::uint64_t); const BioContainmentData* find(std::uint64_t) const; std::vector<BioContainmentData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BioContainmentData> data_; };
}

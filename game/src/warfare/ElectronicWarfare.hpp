#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track jamming, spoofing, detection, encryption, countermeasures, sensor degradation, and data-link disruption.
struct ElectronicWarfareOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ElectronicWarfareData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ElectronicWarfareStore { public: bool apply(const ElectronicWarfareOp&); bool erase(std::uint64_t); const ElectronicWarfareData* find(std::uint64_t) const; std::vector<ElectronicWarfareData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ElectronicWarfareData> data_; };
}

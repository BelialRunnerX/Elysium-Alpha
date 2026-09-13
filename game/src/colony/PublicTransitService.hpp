#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate passenger demand, schedules, capacity, congestion, accessibility, maintenance, and service quality.
struct PublicTransitServiceOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PublicTransitServiceData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PublicTransitServiceStore { public: bool apply(const PublicTransitServiceOp&); bool erase(std::uint64_t); const PublicTransitServiceData* find(std::uint64_t) const; std::vector<PublicTransitServiceData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PublicTransitServiceData> data_; };
}

#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track carried craft, launch/recovery queues, deck capacity, maintenance, armament, pilots, and sortie generation.
struct CarrierOperationsOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CarrierOperationsData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CarrierOperationsStore { public: bool apply(const CarrierOperationsOp&); bool erase(std::uint64_t); const CarrierOperationsData* find(std::uint64_t) const; std::vector<CarrierOperationsData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CarrierOperationsData> data_; };
}

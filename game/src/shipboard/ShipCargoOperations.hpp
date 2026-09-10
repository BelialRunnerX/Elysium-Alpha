#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track loading plans, balance, hazardous separation, customs holds, transfer routes, manifests, and priority cargo.
struct ShipCargoOperationsOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipCargoOperationsData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipCargoOperationsStore { public: bool apply(const ShipCargoOperationsOp&); bool erase(std::uint64_t); const ShipCargoOperationsData* find(std::uint64_t) const; std::vector<ShipCargoOperationsData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipCargoOperationsData> data_; };
}

#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track embarked craft, pilots, launch/recovery cycles, maintenance, fuel, ammunition, and sortie planning.
struct CarrierOperationsCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct CarrierOperationsState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class CarrierOperationsSystem { public: bool submit(const CarrierOperationsCommand&); const CarrierOperationsState* find(std::uint64_t) const; std::vector<CarrierOperationsState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CarrierOperationsState> map_; };
}

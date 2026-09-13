#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent large mobile base/colony ships with residents, industry, storage, utilities, agriculture, and strategic relocation.
struct MobileBaseShipCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct MobileBaseShipState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class MobileBaseShipSystem { public: bool submit(const MobileBaseShipCommand&); const MobileBaseShipState* find(std::uint64_t) const; std::vector<MobileBaseShipState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MobileBaseShipState> map_; };
}

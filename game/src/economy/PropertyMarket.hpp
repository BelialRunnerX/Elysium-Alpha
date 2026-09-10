#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track stable property/room/workshop/claim lease and sale offers without embedding dense world ownership.
struct PropertyMarketCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct PropertyMarketState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class PropertyMarketSystem { public: bool submit(const PropertyMarketCommand&); const PropertyMarketState* find(std::uint64_t) const; std::vector<PropertyMarketState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PropertyMarketState> map_; };
}

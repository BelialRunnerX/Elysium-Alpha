#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Price wrecks, scrap, components, artifacts, claims, towing, hazards, and recovered modules from market context.
struct SalvageMarketCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct SalvageMarketState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class SalvageMarketSystem { public: bool submit(const SalvageMarketCommand&); const SalvageMarketState* find(std::uint64_t) const; std::vector<SalvageMarketState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SalvageMarketState> map_; };
}

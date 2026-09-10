#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate job vacancies, worker skills, wages, migration pull, training needs, and automation substitution.
struct LaborMarketCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct LaborMarketState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class LaborMarketSystem { public: bool submit(const LaborMarketCommand&); const LaborMarketState* find(std::uint64_t) const; std::vector<LaborMarketState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LaborMarketState> map_; };
}

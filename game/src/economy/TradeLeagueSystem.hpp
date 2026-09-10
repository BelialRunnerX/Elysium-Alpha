#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent merchant leagues, shared routes, convoy defense, dues, arbitration, and market access agreements.
struct TradeLeagueSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct TradeLeagueSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class TradeLeagueSystemSystem { public: bool submit(const TradeLeagueSystemCommand&); const TradeLeagueSystemState* find(std::uint64_t) const; std::vector<TradeLeagueSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TradeLeagueSystemState> map_; };
}

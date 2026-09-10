#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent authored Praetor boss abilities, phases, telegraphs, cooldowns, counters, and encounter-state requirements.
struct PraetorAbilitySystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct PraetorAbilitySystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class PraetorAbilitySystemSystem { public: bool submit(const PraetorAbilitySystemCommand&); const PraetorAbilitySystemState* find(std::uint64_t) const; std::vector<PraetorAbilitySystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PraetorAbilitySystemState> map_; };
}

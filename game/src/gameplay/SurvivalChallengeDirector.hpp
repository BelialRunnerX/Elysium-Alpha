#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate optional escalating survival objectives from environment, supplies, route, and settlement context.
struct SurvivalChallengeDirectorCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct SurvivalChallengeDirectorState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class SurvivalChallengeDirectorSystem { public: bool submit(const SurvivalChallengeDirectorCommand&); const SurvivalChallengeDirectorState* find(std::uint64_t) const; std::vector<SurvivalChallengeDirectorState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SurvivalChallengeDirectorState> map_; };
}

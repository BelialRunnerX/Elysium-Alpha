#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate terrain, resource, hazard, weather, anomaly, and structure survey routes over stable surface addresses.
struct SurveyDroneAICommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct SurveyDroneAIState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class SurveyDroneAISystem { public: bool submit(const SurveyDroneAICommand&); const SurveyDroneAIState* find(std::uint64_t) const; std::vector<SurveyDroneAIState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SurveyDroneAIState> map_; };
}

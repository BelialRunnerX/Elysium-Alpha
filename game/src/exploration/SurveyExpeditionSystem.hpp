#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Plan multi-day surface/orbital expeditions with objectives, supplies, route risk, camps, samples, and return requirements.
struct SurveyExpeditionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SurveyExpeditionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SurveyExpeditionSystemStore { public: bool apply(const SurveyExpeditionSystemOp&); bool erase(std::uint64_t); const SurveyExpeditionSystemData* find(std::uint64_t) const; std::vector<SurveyExpeditionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SurveyExpeditionSystemData> data_; };
}

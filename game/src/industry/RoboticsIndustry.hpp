#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent drone frames, actuators, control cores, sensors, tools, repair parts, and autonomous-unit assembly.
struct RoboticsIndustryOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RoboticsIndustryData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RoboticsIndustryStore { public: bool apply(const RoboticsIndustryOp&); bool erase(std::uint64_t); const RoboticsIndustryData* find(std::uint64_t) const; std::vector<RoboticsIndustryData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RoboticsIndustryData> data_; };
}

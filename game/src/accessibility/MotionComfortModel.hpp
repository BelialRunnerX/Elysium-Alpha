#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent camera shake, FOV motion, head bob, acceleration, flash, blur, and vehicle-motion comfort preferences.
struct MotionComfortModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MotionComfortModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MotionComfortModelStore { public: bool apply(const MotionComfortModelOp&); bool erase(std::uint64_t); const MotionComfortModelData* find(std::uint64_t) const; std::vector<MotionComfortModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MotionComfortModelData> data_; };
}

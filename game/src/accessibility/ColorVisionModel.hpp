#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent renderer-neutral color-vision remapping and semantic signal redundancy preferences.
struct ColorVisionModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ColorVisionModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ColorVisionModelStore { public: bool apply(const ColorVisionModelOp&); bool erase(std::uint64_t); const ColorVisionModelData* find(std::uint64_t) const; std::vector<ColorVisionModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ColorVisionModelData> data_; };
}

#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent fibers, fabrics, insulation, filters, uniforms, soft armor, medical textiles, and habitat furnishing production.
struct TextileIndustryOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TextileIndustryData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TextileIndustryStore { public: bool apply(const TextileIndustryOp&); bool erase(std::uint64_t); const TextileIndustryData* find(std::uint64_t) const; std::vector<TextileIndustryData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TextileIndustryData> data_; };
}

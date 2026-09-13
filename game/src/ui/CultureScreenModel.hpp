#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build culture, language, traditions, subcultures, assimilation, revival, events, and historical context projections.
struct CultureScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CultureScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CultureScreenModelStore { public: bool apply(const CultureScreenModelOp&); bool erase(std::uint64_t); const CultureScreenModelData* find(std::uint64_t) const; std::vector<CultureScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CultureScreenModelData> data_; };
}

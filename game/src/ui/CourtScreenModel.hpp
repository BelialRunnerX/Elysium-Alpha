#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build Court factions, favor, audiences, commissions, titles, laws, taxes, intrigues, and consequence projections.
struct CourtScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CourtScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CourtScreenModelStore { public: bool apply(const CourtScreenModelOp&); bool erase(std::uint64_t); const CourtScreenModelData* find(std::uint64_t) const; std::vector<CourtScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CourtScreenModelData> data_; };
}

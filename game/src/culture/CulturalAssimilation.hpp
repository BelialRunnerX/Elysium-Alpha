#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track adoption/resistance of language, customs, law, cuisine, identity, and institutions across mixed populations.
struct CulturalAssimilationOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CulturalAssimilationData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CulturalAssimilationStore { public: bool apply(const CulturalAssimilationOp&); bool erase(std::uint64_t); const CulturalAssimilationData* find(std::uint64_t) const; std::vector<CulturalAssimilationData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CulturalAssimilationData> data_; };
}

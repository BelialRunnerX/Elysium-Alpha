#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track regional radiation weather affecting EVA, ships, electronics, habitats, and route planning.
struct CosmicRadiationWeatherOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CosmicRadiationWeatherData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CosmicRadiationWeatherStore { public: bool apply(const CosmicRadiationWeatherOp&); bool erase(std::uint64_t); const CosmicRadiationWeatherData* find(std::uint64_t) const; std::vector<CosmicRadiationWeatherData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CosmicRadiationWeatherData> data_; };
}

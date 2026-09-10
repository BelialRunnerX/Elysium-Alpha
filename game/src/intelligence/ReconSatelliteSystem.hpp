#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track orbital reconnaissance assets, coverage windows, sensor modes, resolution, weather obstruction, and tasking.
struct ReconSatelliteSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ReconSatelliteSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ReconSatelliteSystemStore { public: bool apply(const ReconSatelliteSystemOp&); bool erase(std::uint64_t); const ReconSatelliteSystemData* find(std::uint64_t) const; std::vector<ReconSatelliteSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ReconSatelliteSystemData> data_; };
}

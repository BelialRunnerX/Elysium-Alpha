#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build renderer-neutral megastructure sectors, damage, lights, construction, utilities, and distance-detail records.
struct MegastructureRenderModelCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct MegastructureRenderModelState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class MegastructureRenderModelSystem { public: bool submit(const MegastructureRenderModelCommand&); const MegastructureRenderModelState* find(std::uint64_t) const; std::vector<MegastructureRenderModelState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MegastructureRenderModelState> map_; };
}

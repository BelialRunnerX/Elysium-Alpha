#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track troop/cargo dropships, manifests, landing zones, fuel, damage, extraction, and tactical mission state.
struct DropshipSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct DropshipSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class DropshipSystemSystem { public: bool submit(const DropshipSystemCommand&); const DropshipSystemState* find(std::uint64_t) const; std::vector<DropshipSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DropshipSystemState> map_; };
}

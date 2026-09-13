#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent mass-driver/elevator/shuttle transfer capacity between surface industry and orbital logistics.
struct OrbitalResourceLinkCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct OrbitalResourceLinkState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class OrbitalResourceLinkSystem { public: bool submit(const OrbitalResourceLinkCommand&); const OrbitalResourceLinkState* find(std::uint64_t) const; std::vector<OrbitalResourceLinkState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,OrbitalResourceLinkState> map_; };
}

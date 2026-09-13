#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Coordinate species reintroduction, habitat repair, invasive control, soil/water cleanup, and biodiversity targets.
struct EcologyRestorationCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct EcologyRestorationState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class EcologyRestorationSystem { public: bool submit(const EcologyRestorationCommand&); const EcologyRestorationState* find(std::uint64_t) const; std::vector<EcologyRestorationState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,EcologyRestorationState> map_; };
}

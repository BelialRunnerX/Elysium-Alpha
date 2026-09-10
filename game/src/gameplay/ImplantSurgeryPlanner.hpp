#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate installation/removal surgery requirements from implant type, patient state, staff, facility, and supplies.
struct ImplantSurgeryPlannerCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ImplantSurgeryPlannerState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ImplantSurgeryPlannerSystem { public: bool submit(const ImplantSurgeryPlannerCommand&); const ImplantSurgeryPlannerState* find(std::uint64_t) const; std::vector<ImplantSurgeryPlannerState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ImplantSurgeryPlannerState> map_; };
}

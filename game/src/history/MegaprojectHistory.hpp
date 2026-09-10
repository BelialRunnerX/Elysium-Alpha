#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Record planning, construction, leadership, accidents, milestones, completion, destruction, and legacy of megaprojects.
struct MegaprojectHistoryCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct MegaprojectHistoryState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class MegaprojectHistorySystem { public: bool submit(const MegaprojectHistoryCommand&); const MegaprojectHistoryState* find(std::uint64_t) const; std::vector<MegaprojectHistoryState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MegaprojectHistoryState> map_; };
}

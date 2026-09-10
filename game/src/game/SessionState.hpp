// Intended function: Track current save/session, active system/planet/site, player stable identity, simulation clocks, pause, and transition state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::game {
struct SessionSnapshot {
    std::uint64_t sessionId{};
    std::uint64_t saveId{};
    std::uint64_t systemId{};
    std::uint64_t planetId{};
    std::uint64_t siteId{};
    std::uint64_t tick{};
};
class SessionSnapshotStore {
public:
 bool put(SessionSnapshot v); bool erase(std::uint64_t id);
 [[nodiscard]] const SessionSnapshot* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<SessionSnapshot>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const SessionSnapshot& v) noexcept; std::vector<SessionSnapshot> values_;
};
} // namespace elysium::game

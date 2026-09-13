// Intended function: Schedule orbital lanes, approach windows, congestion, and collision-avoidance reservations.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct OrbitalTrafficCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct OrbitalTrafficState {
    std::uint64_t revision{};
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double accumulated{};
    double pressure{};
    std::uint64_t updatedTick{};
    std::uint32_t mode{};
    std::uint32_t status{};
    bool active{false};
};

struct OrbitalTrafficEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class OrbitalTrafficService {
public:
    bool apply(const OrbitalTrafficCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const OrbitalTrafficState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<OrbitalTrafficState> ordered() const;
    std::vector<OrbitalTrafficEvent> drainEvents();
    void clear();

private:
    OrbitalTrafficState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<OrbitalTrafficState> states_;
    std::vector<OrbitalTrafficEvent> events_;
};

} // namespace elysium::stellar

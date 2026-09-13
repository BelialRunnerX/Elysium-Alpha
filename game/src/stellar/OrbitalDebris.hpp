// Intended function: Track debris density, cleanup, impact risk, and exclusion zones without individual debris actors.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct OrbitalDebrisCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct OrbitalDebrisState {
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

struct OrbitalDebrisEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class OrbitalDebrisService {
public:
    bool apply(const OrbitalDebrisCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const OrbitalDebrisState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<OrbitalDebrisState> ordered() const;
    std::vector<OrbitalDebrisEvent> drainEvents();
    void clear();

private:
    OrbitalDebrisState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<OrbitalDebrisState> states_;
    std::vector<OrbitalDebrisEvent> events_;
};

} // namespace elysium::stellar

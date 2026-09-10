// Intended function: Measure internal cohesion, separatist pressure, shared identity, and crisis resilience.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct CivilizationCohesionCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct CivilizationCohesionState {
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

struct CivilizationCohesionEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class CivilizationCohesionService {
public:
    bool apply(const CivilizationCohesionCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const CivilizationCohesionState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<CivilizationCohesionState> ordered() const;
    std::vector<CivilizationCohesionEvent> drainEvents();
    void clear();

private:
    CivilizationCohesionState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<CivilizationCohesionState> states_;
    std::vector<CivilizationCohesionEvent> events_;
};

} // namespace elysium::civilization

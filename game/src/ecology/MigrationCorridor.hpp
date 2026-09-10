// Intended function: Track seasonal wildlife migration corridors and disruption by settlements or disasters.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ecology {

struct MigrationCorridorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct MigrationCorridorState {
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

struct MigrationCorridorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class MigrationCorridorService {
public:
    bool apply(const MigrationCorridorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const MigrationCorridorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<MigrationCorridorState> ordered() const;
    std::vector<MigrationCorridorEvent> drainEvents();
    void clear();

private:
    MigrationCorridorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<MigrationCorridorState> states_;
    std::vector<MigrationCorridorEvent> events_;
};

} // namespace elysium::ecology

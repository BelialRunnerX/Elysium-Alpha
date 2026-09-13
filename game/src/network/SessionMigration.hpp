// Intended function: Represent future host/server migration handoff state using durable identities.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::network {

struct SessionMigrationCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct SessionMigrationState {
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

struct SessionMigrationEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class SessionMigrationService {
public:
    bool apply(const SessionMigrationCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const SessionMigrationState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<SessionMigrationState> ordered() const;
    std::vector<SessionMigrationEvent> drainEvents();
    void clear();

private:
    SessionMigrationState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<SessionMigrationState> states_;
    std::vector<SessionMigrationEvent> events_;
};

} // namespace elysium::network

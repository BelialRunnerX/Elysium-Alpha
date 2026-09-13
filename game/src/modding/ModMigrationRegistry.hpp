// Intended function: Register deterministic mod save migrations by namespace and schema version.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::modding {

struct ModMigrationRegistryCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ModMigrationRegistryState {
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

struct ModMigrationRegistryEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ModMigrationRegistryService {
public:
    bool apply(const ModMigrationRegistryCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ModMigrationRegistryState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ModMigrationRegistryState> ordered() const;
    std::vector<ModMigrationRegistryEvent> drainEvents();
    void clear();

private:
    ModMigrationRegistryState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ModMigrationRegistryState> states_;
    std::vector<ModMigrationRegistryEvent> events_;
};

} // namespace elysium::modding

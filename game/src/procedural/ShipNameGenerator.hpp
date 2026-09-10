// Intended function: Generate culture-aware ship names and registries deterministically from stable identities.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::procedural {

struct ShipNameGeneratorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ShipNameGeneratorState {
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

struct ShipNameGeneratorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ShipNameGeneratorService {
public:
    bool apply(const ShipNameGeneratorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ShipNameGeneratorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ShipNameGeneratorState> ordered() const;
    std::vector<ShipNameGeneratorEvent> drainEvents();
    void clear();

private:
    ShipNameGeneratorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ShipNameGeneratorState> states_;
    std::vector<ShipNameGeneratorEvent> events_;
};

} // namespace elysium::procedural

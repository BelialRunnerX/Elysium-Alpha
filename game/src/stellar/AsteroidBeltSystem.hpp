// Intended function: Model belt regions, resource richness, collision risk, claims, and mining depletion.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct AsteroidBeltSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct AsteroidBeltSystemState {
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

struct AsteroidBeltSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class AsteroidBeltSystemService {
public:
    bool apply(const AsteroidBeltSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const AsteroidBeltSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<AsteroidBeltSystemState> ordered() const;
    std::vector<AsteroidBeltSystemEvent> drainEvents();
    void clear();

private:
    AsteroidBeltSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<AsteroidBeltSystemState> states_;
    std::vector<AsteroidBeltSystemEvent> events_;
};

} // namespace elysium::stellar

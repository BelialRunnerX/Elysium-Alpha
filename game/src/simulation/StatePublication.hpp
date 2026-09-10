// Intended function: Publish immutable reader snapshots after owner-system commit barriers.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::simulation {

struct StatePublicationCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct StatePublicationState {
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

struct StatePublicationEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class StatePublicationService {
public:
    bool apply(const StatePublicationCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const StatePublicationState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<StatePublicationState> ordered() const;
    std::vector<StatePublicationEvent> drainEvents();
    void clear();

private:
    StatePublicationState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<StatePublicationState> states_;
    std::vector<StatePublicationEvent> events_;
};

} // namespace elysium::simulation

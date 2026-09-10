// Intended function: Coordinate transfers among conveyors, cargo rail, drones, trucks, ships, and orbital freight.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::logistics {

struct IntermodalHubCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct IntermodalHubState {
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

struct IntermodalHubEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class IntermodalHubService {
public:
    bool apply(const IntermodalHubCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const IntermodalHubState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<IntermodalHubState> ordered() const;
    std::vector<IntermodalHubEvent> drainEvents();
    void clear();

private:
    IntermodalHubState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<IntermodalHubState> states_;
    std::vector<IntermodalHubEvent> events_;
};

} // namespace elysium::logistics

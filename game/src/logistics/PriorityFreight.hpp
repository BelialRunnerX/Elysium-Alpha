// Intended function: Assign emergency, military, life-support, construction, and commercial freight priorities.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::logistics {

struct PriorityFreightCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct PriorityFreightState {
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

struct PriorityFreightEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class PriorityFreightService {
public:
    bool apply(const PriorityFreightCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const PriorityFreightState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<PriorityFreightState> ordered() const;
    std::vector<PriorityFreightEvent> drainEvents();
    void clear();

private:
    PriorityFreightState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<PriorityFreightState> states_;
    std::vector<PriorityFreightEvent> events_;
};

} // namespace elysium::logistics

// Intended function: Reserve bounded throughput on logistics routes with deterministic priority and release.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::logistics {

struct RouteCapacityCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct RouteCapacityState {
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

struct RouteCapacityEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class RouteCapacityService {
public:
    bool apply(const RouteCapacityCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const RouteCapacityState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<RouteCapacityState> ordered() const;
    std::vector<RouteCapacityEvent> drainEvents();
    void clear();

private:
    RouteCapacityState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<RouteCapacityState> states_;
    std::vector<RouteCapacityEvent> events_;
};

} // namespace elysium::logistics

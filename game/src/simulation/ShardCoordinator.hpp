// Intended function: Coordinate activation, sleeping, promotion, demotion, and transfer among simulation shards.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::simulation {

struct ShardCoordinatorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ShardCoordinatorState {
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

struct ShardCoordinatorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ShardCoordinatorService {
public:
    bool apply(const ShardCoordinatorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ShardCoordinatorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ShardCoordinatorState> ordered() const;
    std::vector<ShardCoordinatorEvent> drainEvents();
    void clear();

private:
    ShardCoordinatorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ShardCoordinatorState> states_;
    std::vector<ShardCoordinatorEvent> events_;
};

} // namespace elysium::simulation

// Intended function: Prioritize systems, planets, anomalies, ruins, and surveys under range and risk constraints.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct ExplorationPlannerAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ExplorationPlannerAIState {
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

struct ExplorationPlannerAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ExplorationPlannerAIService {
public:
    bool apply(const ExplorationPlannerAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ExplorationPlannerAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ExplorationPlannerAIState> ordered() const;
    std::vector<ExplorationPlannerAIEvent> drainEvents();
    void clear();

private:
    ExplorationPlannerAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ExplorationPlannerAIState> states_;
    std::vector<ExplorationPlannerAIEvent> events_;
};

} // namespace elysium::ai

// Intended function: Allocate faction research effort from doctrine, scarcity, threats, opportunities, and prerequisites.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct ResearchPlannerAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ResearchPlannerAIState {
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

struct ResearchPlannerAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ResearchPlannerAIService {
public:
    bool apply(const ResearchPlannerAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ResearchPlannerAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ResearchPlannerAIState> ordered() const;
    std::vector<ResearchPlannerAIEvent> drainEvents();
    void clear();

private:
    ResearchPlannerAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ResearchPlannerAIState> states_;
    std::vector<ResearchPlannerAIEvent> events_;
};

} // namespace elysium::ai

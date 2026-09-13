// Intended function: Detect systemic world situations worth surfacing as missions, alerts, rumors, or Chronicle events.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct NarrativeOpportunityAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct NarrativeOpportunityAIState {
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

struct NarrativeOpportunityAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class NarrativeOpportunityAIService {
public:
    bool apply(const NarrativeOpportunityAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const NarrativeOpportunityAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<NarrativeOpportunityAIState> ordered() const;
    std::vector<NarrativeOpportunityAIEvent> drainEvents();
    void clear();

private:
    NarrativeOpportunityAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<NarrativeOpportunityAIState> states_;
    std::vector<NarrativeOpportunityAIEvent> events_;
};

} // namespace elysium::ai

// Intended function: Select intelligence collection, counterintelligence, infiltration, and deception priorities.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct EspionagePlannerAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct EspionagePlannerAIState {
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

struct EspionagePlannerAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class EspionagePlannerAIService {
public:
    bool apply(const EspionagePlannerAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const EspionagePlannerAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<EspionagePlannerAIState> ordered() const;
    std::vector<EspionagePlannerAIEvent> drainEvents();
    void clear();

private:
    EspionagePlannerAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<EspionagePlannerAIState> states_;
    std::vector<EspionagePlannerAIEvent> events_;
};

} // namespace elysium::ai

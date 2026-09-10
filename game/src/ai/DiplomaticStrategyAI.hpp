// Intended function: Choose negotiation posture, treaties, sanctions, mediation, and alliance priorities.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct DiplomaticStrategyAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DiplomaticStrategyAIState {
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

struct DiplomaticStrategyAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DiplomaticStrategyAIService {
public:
    bool apply(const DiplomaticStrategyAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DiplomaticStrategyAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DiplomaticStrategyAIState> ordered() const;
    std::vector<DiplomaticStrategyAIEvent> drainEvents();
    void clear();

private:
    DiplomaticStrategyAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DiplomaticStrategyAIState> states_;
    std::vector<DiplomaticStrategyAIEvent> events_;
};

} // namespace elysium::ai

// Intended function: Select colony-level service, construction, defense, and economic priorities from pressure signals.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct ColonyGovernorAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ColonyGovernorAIState {
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

struct ColonyGovernorAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ColonyGovernorAIService {
public:
    bool apply(const ColonyGovernorAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ColonyGovernorAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ColonyGovernorAIState> ordered() const;
    std::vector<ColonyGovernorAIEvent> drainEvents();
    void clear();

private:
    ColonyGovernorAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ColonyGovernorAIState> states_;
    std::vector<ColonyGovernorAIEvent> events_;
};

} // namespace elysium::ai

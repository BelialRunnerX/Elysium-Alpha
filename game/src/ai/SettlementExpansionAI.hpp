// Intended function: Plan district growth and satellite outposts from population, jobs, terrain, and service capacity.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct SettlementExpansionAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct SettlementExpansionAIState {
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

struct SettlementExpansionAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class SettlementExpansionAIService {
public:
    bool apply(const SettlementExpansionAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const SettlementExpansionAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<SettlementExpansionAIState> ordered() const;
    std::vector<SettlementExpansionAIEvent> drainEvents();
    void clear();

private:
    SettlementExpansionAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<SettlementExpansionAIState> states_;
    std::vector<SettlementExpansionAIEvent> events_;
};

} // namespace elysium::ai

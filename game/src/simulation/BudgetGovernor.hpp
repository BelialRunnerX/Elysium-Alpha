// Intended function: Allocate deterministic CPU/work budgets across bounded simulation systems by priority.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::simulation {

struct BudgetGovernorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct BudgetGovernorState {
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

struct BudgetGovernorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class BudgetGovernorService {
public:
    bool apply(const BudgetGovernorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const BudgetGovernorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<BudgetGovernorState> ordered() const;
    std::vector<BudgetGovernorEvent> drainEvents();
    void clear();

private:
    BudgetGovernorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<BudgetGovernorState> states_;
    std::vector<BudgetGovernorEvent> events_;
};

} // namespace elysium::simulation

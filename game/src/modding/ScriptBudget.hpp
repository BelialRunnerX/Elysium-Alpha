// Intended function: Track per-mod script instruction/time budgets and deterministic suspension policy.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::modding {

struct ScriptBudgetCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ScriptBudgetState {
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

struct ScriptBudgetEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ScriptBudgetService {
public:
    bool apply(const ScriptBudgetCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ScriptBudgetState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ScriptBudgetState> ordered() const;
    std::vector<ScriptBudgetEvent> drainEvents();
    void clear();

private:
    ScriptBudgetState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ScriptBudgetState> states_;
    std::vector<ScriptBudgetEvent> events_;
};

} // namespace elysium::modding

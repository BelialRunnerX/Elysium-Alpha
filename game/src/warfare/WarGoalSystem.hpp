// Intended function: Track explicit war goals, progress, exhaustion impact, and negotiated settlement value.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::warfare {

struct WarGoalSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct WarGoalSystemState {
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

struct WarGoalSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class WarGoalSystemService {
public:
    bool apply(const WarGoalSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const WarGoalSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<WarGoalSystemState> ordered() const;
    std::vector<WarGoalSystemEvent> drainEvents();
    void clear();

private:
    WarGoalSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<WarGoalSystemState> states_;
    std::vector<WarGoalSystemEvent> events_;
};

} // namespace elysium::warfare

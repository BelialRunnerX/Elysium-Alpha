// Intended function: Select dialogue intents from relationship, faction, mission, danger, and recent-event context.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::narrative {

struct DynamicDialogueCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DynamicDialogueState {
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

struct DynamicDialogueEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DynamicDialogueService {
public:
    bool apply(const DynamicDialogueCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DynamicDialogueState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DynamicDialogueState> ordered() const;
    std::vector<DynamicDialogueEvent> drainEvents();
    void clear();

private:
    DynamicDialogueState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DynamicDialogueState> states_;
    std::vector<DynamicDialogueEvent> events_;
};

} // namespace elysium::narrative

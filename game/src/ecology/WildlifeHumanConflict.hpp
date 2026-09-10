// Intended function: Track crop damage, predation, disease, settlement encounters, and mitigation.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ecology {

struct WildlifeHumanConflictCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct WildlifeHumanConflictState {
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

struct WildlifeHumanConflictEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class WildlifeHumanConflictService {
public:
    bool apply(const WildlifeHumanConflictCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const WildlifeHumanConflictState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<WildlifeHumanConflictState> ordered() const;
    std::vector<WildlifeHumanConflictEvent> drainEvents();
    void clear();

private:
    WildlifeHumanConflictState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<WildlifeHumanConflictState> states_;
    std::vector<WildlifeHumanConflictEvent> events_;
};

} // namespace elysium::ecology

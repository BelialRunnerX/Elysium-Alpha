// Intended function: Propagate discovered technology between settlements and factions under contact and policy constraints.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct TechnologyDiffusionCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct TechnologyDiffusionState {
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

struct TechnologyDiffusionEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class TechnologyDiffusionService {
public:
    bool apply(const TechnologyDiffusionCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const TechnologyDiffusionState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<TechnologyDiffusionState> ordered() const;
    std::vector<TechnologyDiffusionEvent> drainEvents();
    void clear();

private:
    TechnologyDiffusionState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<TechnologyDiffusionState> states_;
    std::vector<TechnologyDiffusionEvent> events_;
};

} // namespace elysium::civilization

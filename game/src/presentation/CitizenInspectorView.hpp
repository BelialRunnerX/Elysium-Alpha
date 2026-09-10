// Intended function: Project citizen needs, skills, relationships, injuries, jobs, equipment, and recent history.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::presentation {

struct CitizenInspectorViewCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct CitizenInspectorViewState {
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

struct CitizenInspectorViewEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class CitizenInspectorViewService {
public:
    bool apply(const CitizenInspectorViewCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const CitizenInspectorViewState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<CitizenInspectorViewState> ordered() const;
    std::vector<CitizenInspectorViewEvent> drainEvents();
    void clear();

private:
    CitizenInspectorViewState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<CitizenInspectorViewState> states_;
    std::vector<CitizenInspectorViewEvent> events_;
};

} // namespace elysium::presentation

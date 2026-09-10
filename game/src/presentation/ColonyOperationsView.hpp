// Intended function: Project settlement jobs, services, alerts, stocks, power, atmosphere, and logistics for UI.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::presentation {

struct ColonyOperationsViewCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ColonyOperationsViewState {
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

struct ColonyOperationsViewEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ColonyOperationsViewService {
public:
    bool apply(const ColonyOperationsViewCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ColonyOperationsViewState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ColonyOperationsViewState> ordered() const;
    std::vector<ColonyOperationsViewEvent> drainEvents();
    void clear();

private:
    ColonyOperationsViewState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ColonyOperationsViewState> states_;
    std::vector<ColonyOperationsViewEvent> events_;
};

} // namespace elysium::presentation

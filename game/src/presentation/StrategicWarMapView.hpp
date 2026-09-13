// Intended function: Project theaters, fronts, fleets, supply corridors, objectives, and warnings for UI.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::presentation {

struct StrategicWarMapViewCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct StrategicWarMapViewState {
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

struct StrategicWarMapViewEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class StrategicWarMapViewService {
public:
    bool apply(const StrategicWarMapViewCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const StrategicWarMapViewState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<StrategicWarMapViewState> ordered() const;
    std::vector<StrategicWarMapViewEvent> drainEvents();
    void clear();

private:
    StrategicWarMapViewState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<StrategicWarMapViewState> states_;
    std::vector<StrategicWarMapViewEvent> events_;
};

} // namespace elysium::presentation

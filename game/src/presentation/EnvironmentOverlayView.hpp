// Intended function: Project atmosphere, heat, radiation, contamination, fluids, fire, and structural hazards.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::presentation {

struct EnvironmentOverlayViewCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct EnvironmentOverlayViewState {
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

struct EnvironmentOverlayViewEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class EnvironmentOverlayViewService {
public:
    bool apply(const EnvironmentOverlayViewCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const EnvironmentOverlayViewState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<EnvironmentOverlayViewState> ordered() const;
    std::vector<EnvironmentOverlayViewEvent> drainEvents();
    void clear();

private:
    EnvironmentOverlayViewState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<EnvironmentOverlayViewState> states_;
    std::vector<EnvironmentOverlayViewEvent> events_;
};

} // namespace elysium::presentation

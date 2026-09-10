// Intended function: Project fleet composition, orders, fuel, damage, cargo, formation, and route state.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::presentation {

struct FleetCommandViewCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct FleetCommandViewState {
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

struct FleetCommandViewEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class FleetCommandViewService {
public:
    bool apply(const FleetCommandViewCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const FleetCommandViewState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<FleetCommandViewState> ordered() const;
    std::vector<FleetCommandViewEvent> drainEvents();
    void clear();

private:
    FleetCommandViewState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<FleetCommandViewState> states_;
    std::vector<FleetCommandViewEvent> events_;
};

} // namespace elysium::presentation

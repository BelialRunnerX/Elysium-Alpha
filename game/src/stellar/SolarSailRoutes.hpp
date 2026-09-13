// Intended function: Plan low-fuel sail routes using star pressure, geometry, travel time, and cargo constraints.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct SolarSailRoutesCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct SolarSailRoutesState {
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

struct SolarSailRoutesEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class SolarSailRoutesService {
public:
    bool apply(const SolarSailRoutesCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const SolarSailRoutesState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<SolarSailRoutesState> ordered() const;
    std::vector<SolarSailRoutesEvent> drainEvents();
    void clear();

private:
    SolarSailRoutesState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<SolarSailRoutesState> states_;
    std::vector<SolarSailRoutesEvent> events_;
};

} // namespace elysium::stellar

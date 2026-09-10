// Intended function: Track hazardous cargo classes, containment requirements, route restrictions, and incidents.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::logistics {

struct HazardousCargoCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct HazardousCargoState {
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

struct HazardousCargoEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class HazardousCargoService {
public:
    bool apply(const HazardousCargoCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const HazardousCargoState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<HazardousCargoState> ordered() const;
    std::vector<HazardousCargoEvent> drainEvents();
    void clear();

private:
    HazardousCargoState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<HazardousCargoState> states_;
    std::vector<HazardousCargoEvent> events_;
};

} // namespace elysium::logistics

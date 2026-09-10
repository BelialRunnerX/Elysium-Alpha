// Intended function: Track potable-water capacity, demand, contamination, reserve margin, and rationing policy.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::colony {

struct WaterAuthorityCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct WaterAuthorityState {
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

struct WaterAuthorityEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class WaterAuthorityService {
public:
    bool apply(const WaterAuthorityCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const WaterAuthorityState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<WaterAuthorityState> ordered() const;
    std::vector<WaterAuthorityEvent> drainEvents();
    void clear();

private:
    WaterAuthorityState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<WaterAuthorityState> states_;
    std::vector<WaterAuthorityEvent> events_;
};

} // namespace elysium::colony

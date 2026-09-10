// Intended function: Aggregate casualties, economic disruption, duration, and legitimacy into faction war exhaustion.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::warfare {

struct WarExhaustionCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct WarExhaustionState {
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

struct WarExhaustionEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class WarExhaustionService {
public:
    bool apply(const WarExhaustionCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const WarExhaustionState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<WarExhaustionState> ordered() const;
    std::vector<WarExhaustionEvent> drainEvents();
    void clear();

private:
    WarExhaustionState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<WarExhaustionState> states_;
    std::vector<WarExhaustionEvent> events_;
};

} // namespace elysium::warfare

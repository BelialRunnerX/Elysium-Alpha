// Intended function: Represent culturally interpreted forecasts without making them authoritative simulation truth.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::narrative {

struct ProphecySystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ProphecySystemState {
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

struct ProphecySystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ProphecySystemService {
public:
    bool apply(const ProphecySystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ProphecySystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ProphecySystemState> ordered() const;
    std::vector<ProphecySystemEvent> drainEvents();
    void clear();

private:
    ProphecySystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ProphecySystemState> states_;
    std::vector<ProphecySystemEvent> events_;
};

} // namespace elysium::narrative

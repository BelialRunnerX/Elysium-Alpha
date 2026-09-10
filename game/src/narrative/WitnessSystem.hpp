// Intended function: Track who witnessed significant events so knowledge and testimony remain bounded.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::narrative {

struct WitnessSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct WitnessSystemState {
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

struct WitnessSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class WitnessSystemService {
public:
    bool apply(const WitnessSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const WitnessSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<WitnessSystemState> ordered() const;
    std::vector<WitnessSystemEvent> drainEvents();
    void clear();

private:
    WitnessSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<WitnessSystemState> states_;
    std::vector<WitnessSystemEvent> events_;
};

} // namespace elysium::narrative

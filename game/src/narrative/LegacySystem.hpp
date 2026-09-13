// Intended function: Aggregate a character or settlement legacy from Chronicle events and durable achievements.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::narrative {

struct LegacySystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct LegacySystemState {
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

struct LegacySystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class LegacySystemService {
public:
    bool apply(const LegacySystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const LegacySystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<LegacySystemState> ordered() const;
    std::vector<LegacySystemEvent> drainEvents();
    void clear();

private:
    LegacySystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<LegacySystemState> states_;
    std::vector<LegacySystemEvent> events_;
};

} // namespace elysium::narrative

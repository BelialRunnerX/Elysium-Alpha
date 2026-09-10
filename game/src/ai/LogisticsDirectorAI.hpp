// Intended function: Allocate strategic logistics capacity among civilian, industrial, military, and emergency demand.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct LogisticsDirectorAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct LogisticsDirectorAIState {
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

struct LogisticsDirectorAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class LogisticsDirectorAIService {
public:
    bool apply(const LogisticsDirectorAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const LogisticsDirectorAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<LogisticsDirectorAIState> ordered() const;
    std::vector<LogisticsDirectorAIEvent> drainEvents();
    void clear();

private:
    LogisticsDirectorAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<LogisticsDirectorAIState> states_;
    std::vector<LogisticsDirectorAIEvent> events_;
};

} // namespace elysium::ai

// Intended function: Track competing interpretations of Chronicle facts among cultures and institutions.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::narrative {

struct HistoricalDebateCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct HistoricalDebateState {
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

struct HistoricalDebateEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class HistoricalDebateService {
public:
    bool apply(const HistoricalDebateCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const HistoricalDebateState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<HistoricalDebateState> ordered() const;
    std::vector<HistoricalDebateEvent> drainEvents();
    void clear();

private:
    HistoricalDebateState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<HistoricalDebateState> states_;
    std::vector<HistoricalDebateEvent> events_;
};

} // namespace elysium::narrative

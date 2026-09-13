// Intended function: Forecast deterministic comet passages, resource windows, hazards, and interception opportunities.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct CometForecastCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct CometForecastState {
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

struct CometForecastEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class CometForecastService {
public:
    bool apply(const CometForecastCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const CometForecastState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<CometForecastState> ordered() const;
    std::vector<CometForecastEvent> drainEvents();
    void clear();

private:
    CometForecastState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<CometForecastState> states_;
    std::vector<CometForecastEvent> events_;
};

} // namespace elysium::stellar

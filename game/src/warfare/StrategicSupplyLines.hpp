// Intended function: Track military supply corridors, throughput, disruption, and army/fleet readiness penalties.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::warfare {

struct StrategicSupplyLinesCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct StrategicSupplyLinesState {
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

struct StrategicSupplyLinesEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class StrategicSupplyLinesService {
public:
    bool apply(const StrategicSupplyLinesCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const StrategicSupplyLinesState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<StrategicSupplyLinesState> ordered() const;
    std::vector<StrategicSupplyLinesEvent> drainEvents();
    void clear();

private:
    StrategicSupplyLinesState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<StrategicSupplyLinesState> states_;
    std::vector<StrategicSupplyLinesEvent> events_;
};

} // namespace elysium::warfare

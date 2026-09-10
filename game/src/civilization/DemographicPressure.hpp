// Intended function: Track population growth, age pressure, dependency ratios, and migration demand across civilizations.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct DemographicPressureCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DemographicPressureState {
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

struct DemographicPressureEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DemographicPressureService {
public:
    bool apply(const DemographicPressureCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DemographicPressureState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DemographicPressureState> ordered() const;
    std::vector<DemographicPressureEvent> drainEvents();
    void clear();

private:
    DemographicPressureState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DemographicPressureState> states_;
    std::vector<DemographicPressureEvent> events_;
};

} // namespace elysium::civilization

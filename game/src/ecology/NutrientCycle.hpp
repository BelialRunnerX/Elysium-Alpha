// Intended function: Track coarse soil/water nutrient flows, depletion, recycling, and ecosystem productivity.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ecology {

struct NutrientCycleCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct NutrientCycleState {
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

struct NutrientCycleEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class NutrientCycleService {
public:
    bool apply(const NutrientCycleCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const NutrientCycleState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<NutrientCycleState> ordered() const;
    std::vector<NutrientCycleEvent> drainEvents();
    void clear();

private:
    NutrientCycleState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<NutrientCycleState> states_;
    std::vector<NutrientCycleEvent> events_;
};

} // namespace elysium::ecology

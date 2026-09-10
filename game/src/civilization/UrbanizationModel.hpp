// Intended function: Aggregate settlement growth into urbanization tiers and infrastructure demand.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct UrbanizationModelCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct UrbanizationModelState {
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

struct UrbanizationModelEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class UrbanizationModelService {
public:
    bool apply(const UrbanizationModelCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const UrbanizationModelState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<UrbanizationModelState> ordered() const;
    std::vector<UrbanizationModelEvent> drainEvents();
    void clear();

private:
    UrbanizationModelState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<UrbanizationModelState> states_;
    std::vector<UrbanizationModelEvent> events_;
};

} // namespace elysium::civilization

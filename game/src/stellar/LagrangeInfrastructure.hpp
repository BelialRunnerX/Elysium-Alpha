// Intended function: Track stable orbital infrastructure positioned around Lagrange regions by durable IDs.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct LagrangeInfrastructureCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct LagrangeInfrastructureState {
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

struct LagrangeInfrastructureEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class LagrangeInfrastructureService {
public:
    bool apply(const LagrangeInfrastructureCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const LagrangeInfrastructureState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<LagrangeInfrastructureState> ordered() const;
    std::vector<LagrangeInfrastructureEvent> drainEvents();
    void clear();

private:
    LagrangeInfrastructureState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<LagrangeInfrastructureState> states_;
    std::vector<LagrangeInfrastructureEvent> events_;
};

} // namespace elysium::stellar

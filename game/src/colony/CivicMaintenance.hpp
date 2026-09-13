// Intended function: Plan recurring maintenance load for roads, utilities, public spaces, and civic assets.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::colony {

struct CivicMaintenanceCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct CivicMaintenanceState {
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

struct CivicMaintenanceEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class CivicMaintenanceService {
public:
    bool apply(const CivicMaintenanceCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const CivicMaintenanceState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<CivicMaintenanceState> ordered() const;
    std::vector<CivicMaintenanceEvent> drainEvents();
    void clear();

private:
    CivicMaintenanceState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<CivicMaintenanceState> states_;
    std::vector<CivicMaintenanceEvent> events_;
};

} // namespace elysium::colony

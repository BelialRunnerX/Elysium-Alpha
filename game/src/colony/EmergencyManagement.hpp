// Intended function: Coordinate disaster readiness, shelters, evacuation capacity, supplies, and incident response.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::colony {

struct EmergencyManagementCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct EmergencyManagementState {
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

struct EmergencyManagementEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class EmergencyManagementService {
public:
    bool apply(const EmergencyManagementCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const EmergencyManagementState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<EmergencyManagementState> ordered() const;
    std::vector<EmergencyManagementEvent> drainEvents();
    void clear();

private:
    EmergencyManagementState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<EmergencyManagementState> states_;
    std::vector<EmergencyManagementEvent> events_;
};

} // namespace elysium::colony

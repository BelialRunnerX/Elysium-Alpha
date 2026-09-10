// Intended function: Track embassy and envoy missions, objectives, security, progress, and negotiated outcomes.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::diplomacy {

struct DiplomaticMissionCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DiplomaticMissionState {
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

struct DiplomaticMissionEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DiplomaticMissionService {
public:
    bool apply(const DiplomaticMissionCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DiplomaticMissionState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DiplomaticMissionState> ordered() const;
    std::vector<DiplomaticMissionEvent> drainEvents();
    void clear();

private:
    DiplomaticMissionState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DiplomaticMissionState> states_;
    std::vector<DiplomaticMissionEvent> events_;
};

} // namespace elysium::diplomacy

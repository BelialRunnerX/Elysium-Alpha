// Intended function: Coordinate third-party mediation offers and de-escalation progress between hostile factions.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::diplomacy {

struct MediationSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct MediationSystemState {
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

struct MediationSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class MediationSystemService {
public:
    bool apply(const MediationSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const MediationSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<MediationSystemState> ordered() const;
    std::vector<MediationSystemEvent> drainEvents();
    void clear();

private:
    MediationSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<MediationSystemState> states_;
    std::vector<MediationSystemEvent> events_;
};

} // namespace elysium::diplomacy

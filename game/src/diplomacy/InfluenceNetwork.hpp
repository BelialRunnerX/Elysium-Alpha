// Intended function: Track soft-power influence across settlements, factions, institutions, and strategic sites.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::diplomacy {

struct InfluenceNetworkCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct InfluenceNetworkState {
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

struct InfluenceNetworkEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class InfluenceNetworkService {
public:
    bool apply(const InfluenceNetworkCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const InfluenceNetworkState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<InfluenceNetworkState> ordered() const;
    std::vector<InfluenceNetworkEvent> drainEvents();
    void clear();

private:
    InfluenceNetworkState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<InfluenceNetworkState> states_;
    std::vector<InfluenceNetworkEvent> events_;
};

} // namespace elysium::diplomacy

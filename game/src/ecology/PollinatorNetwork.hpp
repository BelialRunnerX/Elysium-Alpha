// Intended function: Track pollinator abundance and crop/wild-flora dependencies across bounded habitat regions.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ecology {

struct PollinatorNetworkCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct PollinatorNetworkState {
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

struct PollinatorNetworkEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class PollinatorNetworkService {
public:
    bool apply(const PollinatorNetworkCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const PollinatorNetworkState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<PollinatorNetworkState> ordered() const;
    std::vector<PollinatorNetworkEvent> drainEvents();
    void clear();

private:
    PollinatorNetworkState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<PollinatorNetworkState> states_;
    std::vector<PollinatorNetworkEvent> events_;
};

} // namespace elysium::ecology

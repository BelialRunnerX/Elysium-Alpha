// Intended function: Track remote tick estimates, jitter windows, interpolation delay, and clock corrections.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::network {

struct NetworkTimeCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct NetworkTimeState {
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

struct NetworkTimeEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class NetworkTimeService {
public:
    bool apply(const NetworkTimeCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const NetworkTimeState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<NetworkTimeState> ordered() const;
    std::vector<NetworkTimeEvent> drainEvents();
    void clear();

private:
    NetworkTimeState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<NetworkTimeState> states_;
    std::vector<NetworkTimeEvent> events_;
};

} // namespace elysium::network

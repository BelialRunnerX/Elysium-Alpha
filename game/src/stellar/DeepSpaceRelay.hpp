// Intended function: Track relay coverage, latency, bandwidth, outages, and route-dependent communications.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct DeepSpaceRelayCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DeepSpaceRelayState {
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

struct DeepSpaceRelayEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DeepSpaceRelayService {
public:
    bool apply(const DeepSpaceRelayCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DeepSpaceRelayState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DeepSpaceRelayState> ordered() const;
    std::vector<DeepSpaceRelayEvent> drainEvents();
    void clear();

private:
    DeepSpaceRelayState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DeepSpaceRelayState> states_;
    std::vector<DeepSpaceRelayEvent> events_;
};

} // namespace elysium::stellar

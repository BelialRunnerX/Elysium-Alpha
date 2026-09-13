// Intended function: Aggregate settlement transit capacity, congestion, commute burden, and service reliability.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::colony {

struct PublicTransitCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct PublicTransitState {
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

struct PublicTransitEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class PublicTransitService {
public:
    bool apply(const PublicTransitCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const PublicTransitState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<PublicTransitState> ordered() const;
    std::vector<PublicTransitEvent> drainEvents();
    void clear();

private:
    PublicTransitState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<PublicTransitState> states_;
    std::vector<PublicTransitEvent> events_;
};

} // namespace elysium::colony

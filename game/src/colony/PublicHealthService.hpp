// Intended function: Aggregate sanitation, clinic capacity, vaccination, exposure, and population health pressure.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::colony {

struct PublicHealthServiceCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct PublicHealthServiceState {
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

struct PublicHealthServiceEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class PublicHealthServiceService {
public:
    bool apply(const PublicHealthServiceCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const PublicHealthServiceState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<PublicHealthServiceState> ordered() const;
    std::vector<PublicHealthServiceEvent> drainEvents();
    void clear();

private:
    PublicHealthServiceState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<PublicHealthServiceState> states_;
    std::vector<PublicHealthServiceEvent> events_;
};

} // namespace elysium::colony

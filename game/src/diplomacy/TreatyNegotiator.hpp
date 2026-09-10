// Intended function: Build and track multi-term treaty proposals with concessions, value balance, and expiration.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::diplomacy {

struct TreatyNegotiatorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct TreatyNegotiatorState {
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

struct TreatyNegotiatorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class TreatyNegotiatorService {
public:
    bool apply(const TreatyNegotiatorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const TreatyNegotiatorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<TreatyNegotiatorState> ordered() const;
    std::vector<TreatyNegotiatorEvent> drainEvents();
    void clear();

private:
    TreatyNegotiatorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<TreatyNegotiatorState> states_;
    std::vector<TreatyNegotiatorEvent> events_;
};

} // namespace elysium::diplomacy

// Intended function: Represent authoritative command acceptance/rejection without exposing transport-specific APIs.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::network {

struct ServerAuthorityCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ServerAuthorityState {
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

struct ServerAuthorityEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ServerAuthorityService {
public:
    bool apply(const ServerAuthorityCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ServerAuthorityState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ServerAuthorityState> ordered() const;
    std::vector<ServerAuthorityEvent> drainEvents();
    void clear();

private:
    ServerAuthorityState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ServerAuthorityState> states_;
    std::vector<ServerAuthorityEvent> events_;
};

} // namespace elysium::network

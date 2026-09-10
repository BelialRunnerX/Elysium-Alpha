// Intended function: Represent command rate, ownership, capability, and validation policy boundaries.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::network {

struct NetworkSecurityPolicyCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct NetworkSecurityPolicyState {
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

struct NetworkSecurityPolicyEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class NetworkSecurityPolicyService {
public:
    bool apply(const NetworkSecurityPolicyCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const NetworkSecurityPolicyState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<NetworkSecurityPolicyState> ordered() const;
    std::vector<NetworkSecurityPolicyEvent> drainEvents();
    void clear();

private:
    NetworkSecurityPolicyState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<NetworkSecurityPolicyState> states_;
    std::vector<NetworkSecurityPolicyEvent> events_;
};

} // namespace elysium::network

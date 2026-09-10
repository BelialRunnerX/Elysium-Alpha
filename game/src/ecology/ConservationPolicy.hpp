// Intended function: Track protected habitats, harvesting limits, restoration projects, and enforcement.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ecology {

struct ConservationPolicyCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ConservationPolicyState {
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

struct ConservationPolicyEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ConservationPolicyService {
public:
    bool apply(const ConservationPolicyCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ConservationPolicyState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ConservationPolicyState> ordered() const;
    std::vector<ConservationPolicyEvent> drainEvents();
    void clear();

private:
    ConservationPolicyState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ConservationPolicyState> states_;
    std::vector<ConservationPolicyEvent> events_;
};

} // namespace elysium::ecology

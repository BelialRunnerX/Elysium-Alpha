// Intended function: Propagate witnessed diplomatic behavior into regional reputation without global scans.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::diplomacy {

struct ReputationPropagationCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ReputationPropagationState {
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

struct ReputationPropagationEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ReputationPropagationService {
public:
    bool apply(const ReputationPropagationCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ReputationPropagationState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ReputationPropagationState> ordered() const;
    std::vector<ReputationPropagationEvent> drainEvents();
    void clear();

private:
    ReputationPropagationState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ReputationPropagationState> states_;
    std::vector<ReputationPropagationEvent> events_;
};

} // namespace elysium::diplomacy

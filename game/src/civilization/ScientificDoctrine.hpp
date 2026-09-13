// Intended function: Track research investment priorities, risk appetite, and dissemination policy.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct ScientificDoctrineCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ScientificDoctrineState {
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

struct ScientificDoctrineEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ScientificDoctrineService {
public:
    bool apply(const ScientificDoctrineCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ScientificDoctrineState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ScientificDoctrineState> ordered() const;
    std::vector<ScientificDoctrineEvent> drainEvents();
    void clear();

private:
    ScientificDoctrineState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ScientificDoctrineState> states_;
    std::vector<ScientificDoctrineEvent> events_;
};

} // namespace elysium::civilization

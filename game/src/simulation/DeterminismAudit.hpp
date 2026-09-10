// Intended function: Record input/order hashes used to diagnose nondeterministic simulation divergence.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::simulation {

struct DeterminismAuditCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DeterminismAuditState {
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

struct DeterminismAuditEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DeterminismAuditService {
public:
    bool apply(const DeterminismAuditCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DeterminismAuditState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DeterminismAuditState> ordered() const;
    std::vector<DeterminismAuditEvent> drainEvents();
    void clear();

private:
    DeterminismAuditState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DeterminismAuditState> states_;
    std::vector<DeterminismAuditEvent> events_;
};

} // namespace elysium::simulation

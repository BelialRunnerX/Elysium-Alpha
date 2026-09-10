// Intended function: Compare deterministic hashes and state summaries to localize multiplayer divergence.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::network {

struct DesyncDiagnosticsCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DesyncDiagnosticsState {
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

struct DesyncDiagnosticsEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DesyncDiagnosticsService {
public:
    bool apply(const DesyncDiagnosticsCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DesyncDiagnosticsState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DesyncDiagnosticsState> ordered() const;
    std::vector<DesyncDiagnosticsEvent> drainEvents();
    void clear();

private:
    DesyncDiagnosticsState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DesyncDiagnosticsState> states_;
    std::vector<DesyncDiagnosticsEvent> events_;
};

} // namespace elysium::network

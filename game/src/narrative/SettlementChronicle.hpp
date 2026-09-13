// Intended function: Build settlement-facing timelines and milestones from durable Chronicle references.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::narrative {

struct SettlementChronicleCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct SettlementChronicleState {
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

struct SettlementChronicleEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class SettlementChronicleService {
public:
    bool apply(const SettlementChronicleCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const SettlementChronicleState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<SettlementChronicleState> ordered() const;
    std::vector<SettlementChronicleEvent> drainEvents();
    void clear();

private:
    SettlementChronicleState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<SettlementChronicleState> states_;
    std::vector<SettlementChronicleEvent> events_;
};

} // namespace elysium::narrative

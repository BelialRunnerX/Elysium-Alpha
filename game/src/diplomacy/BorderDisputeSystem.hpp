// Intended function: Track competing territorial claims, incidents, escalation, and negotiated border settlements.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::diplomacy {

struct BorderDisputeSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct BorderDisputeSystemState {
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

struct BorderDisputeSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class BorderDisputeSystemService {
public:
    bool apply(const BorderDisputeSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const BorderDisputeSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<BorderDisputeSystemState> ordered() const;
    std::vector<BorderDisputeSystemEvent> drainEvents();
    void clear();

private:
    BorderDisputeSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<BorderDisputeSystemState> states_;
    std::vector<BorderDisputeSystemEvent> events_;
};

} // namespace elysium::diplomacy

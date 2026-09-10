// Intended function: Track resistance cells, support, suppression pressure, sabotage potential, and escalation.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::warfare {

struct InsurgencySystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct InsurgencySystemState {
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

struct InsurgencySystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class InsurgencySystemService {
public:
    bool apply(const InsurgencySystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const InsurgencySystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<InsurgencySystemState> ordered() const;
    std::vector<InsurgencySystemEvent> drainEvents();
    void clear();

private:
    InsurgencySystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<InsurgencySystemState> states_;
    std::vector<InsurgencySystemEvent> events_;
};

} // namespace elysium::warfare

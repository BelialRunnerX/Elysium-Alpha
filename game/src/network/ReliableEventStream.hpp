// Intended function: Track ordered reliable gameplay events independently from snapshot replication.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::network {

struct ReliableEventStreamCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ReliableEventStreamState {
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

struct ReliableEventStreamEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ReliableEventStreamService {
public:
    bool apply(const ReliableEventStreamCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ReliableEventStreamState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ReliableEventStreamState> ordered() const;
    std::vector<ReliableEventStreamEvent> drainEvents();
    void clear();

private:
    ReliableEventStreamState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ReliableEventStreamState> states_;
    std::vector<ReliableEventStreamEvent> events_;
};

} // namespace elysium::network

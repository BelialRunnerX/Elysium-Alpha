// Intended function: Track civilization-level strategic resource priorities and substitution pressure.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct ResourceDoctrineCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ResourceDoctrineState {
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

struct ResourceDoctrineEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ResourceDoctrineService {
public:
    bool apply(const ResourceDoctrineCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ResourceDoctrineState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ResourceDoctrineState> ordered() const;
    std::vector<ResourceDoctrineEvent> drainEvents();
    void clear();

private:
    ResourceDoctrineState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ResourceDoctrineState> states_;
    std::vector<ResourceDoctrineEvent> events_;
};

} // namespace elysium::civilization

// Intended function: Generate layered ruins from abandoned settlement roles, destruction causes, and elapsed time.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::procedural {

struct RuinsGeneratorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct RuinsGeneratorState {
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

struct RuinsGeneratorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class RuinsGeneratorService {
public:
    bool apply(const RuinsGeneratorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const RuinsGeneratorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<RuinsGeneratorState> ordered() const;
    std::vector<RuinsGeneratorEvent> drainEvents();
    void clear();

private:
    RuinsGeneratorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<RuinsGeneratorState> states_;
    std::vector<RuinsGeneratorEvent> events_;
};

} // namespace elysium::procedural

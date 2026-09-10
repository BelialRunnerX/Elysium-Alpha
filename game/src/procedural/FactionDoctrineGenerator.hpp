// Intended function: Generate faction military/economic/diplomatic doctrine mixes from history and environment.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::procedural {

struct FactionDoctrineGeneratorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct FactionDoctrineGeneratorState {
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

struct FactionDoctrineGeneratorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class FactionDoctrineGeneratorService {
public:
    bool apply(const FactionDoctrineGeneratorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const FactionDoctrineGeneratorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<FactionDoctrineGeneratorState> ordered() const;
    std::vector<FactionDoctrineGeneratorEvent> drainEvents();
    void clear();

private:
    FactionDoctrineGeneratorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<FactionDoctrineGeneratorState> states_;
    std::vector<FactionDoctrineGeneratorEvent> events_;
};

} // namespace elysium::procedural

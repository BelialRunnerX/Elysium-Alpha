// Intended function: Track star age, luminosity, activity, and long-horizon stellar lifecycle states.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct StellarEvolutionCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct StellarEvolutionState {
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

struct StellarEvolutionEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class StellarEvolutionService {
public:
    bool apply(const StellarEvolutionCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const StellarEvolutionState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<StellarEvolutionState> ordered() const;
    std::vector<StellarEvolutionEvent> drainEvents();
    void clear();

private:
    StellarEvolutionState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<StellarEvolutionState> states_;
    std::vector<StellarEvolutionEvent> events_;
};

} // namespace elysium::stellar

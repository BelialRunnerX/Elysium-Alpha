// Intended function: Represent supply, demand, financial, war, and disaster shocks with deterministic decay.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::economy {

struct EconomicShockCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct EconomicShockState {
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

struct EconomicShockEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class EconomicShockService {
public:
    bool apply(const EconomicShockCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const EconomicShockState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<EconomicShockState> ordered() const;
    std::vector<EconomicShockEvent> drainEvents();
    void clear();

private:
    EconomicShockState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<EconomicShockState> states_;
    std::vector<EconomicShockEvent> events_;
};

} // namespace elysium::economy

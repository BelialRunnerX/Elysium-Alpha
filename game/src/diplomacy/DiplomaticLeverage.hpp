// Intended function: Aggregate favors, debts, dependency, secrets, and military pressure into negotiation leverage.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::diplomacy {

struct DiplomaticLeverageCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DiplomaticLeverageState {
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

struct DiplomaticLeverageEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DiplomaticLeverageService {
public:
    bool apply(const DiplomaticLeverageCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DiplomaticLeverageState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DiplomaticLeverageState> ordered() const;
    std::vector<DiplomaticLeverageEvent> drainEvents();
    void clear();

private:
    DiplomaticLeverageState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DiplomaticLeverageState> states_;
    std::vector<DiplomaticLeverageEvent> events_;
};

} // namespace elysium::diplomacy

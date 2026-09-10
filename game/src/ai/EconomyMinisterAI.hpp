// Intended function: Adjust taxes, subsidies, reserves, procurement, and trade priorities from economic indicators.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct EconomyMinisterAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct EconomyMinisterAIState {
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

struct EconomyMinisterAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class EconomyMinisterAIService {
public:
    bool apply(const EconomyMinisterAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const EconomyMinisterAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<EconomyMinisterAIState> ordered() const;
    std::vector<EconomyMinisterAIEvent> drainEvents();
    void clear();

private:
    EconomyMinisterAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<EconomyMinisterAIState> states_;
    std::vector<EconomyMinisterAIEvent> events_;
};

} // namespace elysium::ai

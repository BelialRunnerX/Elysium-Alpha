// Intended function: Project prices, wages, trade, taxes, budgets, shortages, and production indicators for UI.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::presentation {

struct EconomyViewCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct EconomyViewState {
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

struct EconomyViewEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class EconomyViewService {
public:
    bool apply(const EconomyViewCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const EconomyViewState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<EconomyViewState> ordered() const;
    std::vector<EconomyViewEvent> drainEvents();
    void clear();

private:
    EconomyViewState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<EconomyViewState> states_;
    std::vector<EconomyViewEvent> events_;
};

} // namespace elysium::presentation

// Intended function: Project relations, treaties, incidents, leverage, negotiations, and reputation for UI.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::presentation {

struct DiplomacyViewCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DiplomacyViewState {
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

struct DiplomacyViewEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DiplomacyViewService {
public:
    bool apply(const DiplomacyViewCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DiplomacyViewState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DiplomacyViewState> ordered() const;
    std::vector<DiplomacyViewEvent> drainEvents();
    void clear();

private:
    DiplomacyViewState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DiplomacyViewState> states_;
    std::vector<DiplomacyViewEvent> events_;
};

} // namespace elysium::presentation

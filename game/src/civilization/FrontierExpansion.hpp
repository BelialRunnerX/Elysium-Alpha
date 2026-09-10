// Intended function: Score frontier systems for colonization, supply reach, danger, and political appetite.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct FrontierExpansionCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct FrontierExpansionState {
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

struct FrontierExpansionEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class FrontierExpansionService {
public:
    bool apply(const FrontierExpansionCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const FrontierExpansionState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<FrontierExpansionState> ordered() const;
    std::vector<FrontierExpansionEvent> drainEvents();
    void clear();

private:
    FrontierExpansionState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<FrontierExpansionState> states_;
    std::vector<FrontierExpansionEvent> events_;
};

} // namespace elysium::civilization

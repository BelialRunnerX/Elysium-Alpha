// Intended function: Project discovered, available, blocked, experimental, and faction-shared research nodes.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::presentation {

struct ResearchTreeViewCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ResearchTreeViewState {
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

struct ResearchTreeViewEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ResearchTreeViewService {
public:
    bool apply(const ResearchTreeViewCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ResearchTreeViewState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ResearchTreeViewState> ordered() const;
    std::vector<ResearchTreeViewEvent> drainEvents();
    void clear();

private:
    ResearchTreeViewState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ResearchTreeViewState> states_;
    std::vector<ResearchTreeViewEvent> events_;
};

} // namespace elysium::presentation

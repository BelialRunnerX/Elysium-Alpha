// Intended function: Apply deterministic content patch layering with provenance and conflict diagnostics.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::modding {

struct ContentPatchPlanCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ContentPatchPlanState {
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

struct ContentPatchPlanEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ContentPatchPlanService {
public:
    bool apply(const ContentPatchPlanCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ContentPatchPlanState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ContentPatchPlanState> ordered() const;
    std::vector<ContentPatchPlanEvent> drainEvents();
    void clear();

private:
    ContentPatchPlanState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ContentPatchPlanState> states_;
    std::vector<ContentPatchPlanEvent> events_;
};

} // namespace elysium::modding

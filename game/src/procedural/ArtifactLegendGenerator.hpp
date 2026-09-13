// Intended function: Generate legends and provenance summaries around durable artifact history.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::procedural {

struct ArtifactLegendGeneratorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ArtifactLegendGeneratorState {
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

struct ArtifactLegendGeneratorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ArtifactLegendGeneratorService {
public:
    bool apply(const ArtifactLegendGeneratorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ArtifactLegendGeneratorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ArtifactLegendGeneratorState> ordered() const;
    std::vector<ArtifactLegendGeneratorEvent> drainEvents();
    void clear();

private:
    ArtifactLegendGeneratorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ArtifactLegendGeneratorState> states_;
    std::vector<ArtifactLegendGeneratorEvent> events_;
};

} // namespace elysium::procedural

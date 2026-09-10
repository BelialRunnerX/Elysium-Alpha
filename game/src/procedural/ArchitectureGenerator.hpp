// Intended function: Generate settlement architectural motifs from culture, climate, materials, and technology.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::procedural {

struct ArchitectureGeneratorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ArchitectureGeneratorState {
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

struct ArchitectureGeneratorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ArchitectureGeneratorService {
public:
    bool apply(const ArchitectureGeneratorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ArchitectureGeneratorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ArchitectureGeneratorState> ordered() const;
    std::vector<ArchitectureGeneratorEvent> drainEvents();
    void clear();

private:
    ArchitectureGeneratorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ArchitectureGeneratorState> states_;
    std::vector<ArchitectureGeneratorEvent> events_;
};

} // namespace elysium::procedural

// Intended function: Track cross-settlement cultural influence, assimilation, resistance, and hybridization pressure.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct CulturalDiffusionCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct CulturalDiffusionState {
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

struct CulturalDiffusionEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class CulturalDiffusionService {
public:
    bool apply(const CulturalDiffusionCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const CulturalDiffusionState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<CulturalDiffusionState> ordered() const;
    std::vector<CulturalDiffusionEvent> drainEvents();
    void clear();

private:
    CulturalDiffusionState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<CulturalDiffusionState> states_;
    std::vector<CulturalDiffusionEvent> events_;
};

} // namespace elysium::civilization

// Intended function: Measure alliance willingness to honor mutual obligations under cost and threat pressure.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::diplomacy {

struct AllianceCohesionCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct AllianceCohesionState {
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

struct AllianceCohesionEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class AllianceCohesionService {
public:
    bool apply(const AllianceCohesionCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const AllianceCohesionState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<AllianceCohesionState> ordered() const;
    std::vector<AllianceCohesionEvent> drainEvents();
    void clear();

private:
    AllianceCohesionState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<AllianceCohesionState> states_;
    std::vector<AllianceCohesionEvent> events_;
};

} // namespace elysium::diplomacy

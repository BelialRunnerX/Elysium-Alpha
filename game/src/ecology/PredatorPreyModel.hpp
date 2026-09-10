// Intended function: Track aggregate predator/prey pressure, carrying capacity, hunting success, and population response.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ecology {

struct PredatorPreyModelCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct PredatorPreyModelState {
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

struct PredatorPreyModelEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class PredatorPreyModelService {
public:
    bool apply(const PredatorPreyModelCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const PredatorPreyModelState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<PredatorPreyModelState> ordered() const;
    std::vector<PredatorPreyModelEvent> drainEvents();
    void clear();

private:
    PredatorPreyModelState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<PredatorPreyModelState> states_;
    std::vector<PredatorPreyModelEvent> events_;
};

} // namespace elysium::ecology

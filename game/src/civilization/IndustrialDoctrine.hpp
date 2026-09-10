// Intended function: Choose strategic industrial emphasis from military, expansion, scarcity, and prosperity pressures.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct IndustrialDoctrineCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct IndustrialDoctrineState {
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

struct IndustrialDoctrineEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class IndustrialDoctrineService {
public:
    bool apply(const IndustrialDoctrineCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const IndustrialDoctrineState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<IndustrialDoctrineState> ordered() const;
    std::vector<IndustrialDoctrineEvent> drainEvents();
    void clear();

private:
    IndustrialDoctrineState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<IndustrialDoctrineState> states_;
    std::vector<IndustrialDoctrineEvent> events_;
};

} // namespace elysium::civilization

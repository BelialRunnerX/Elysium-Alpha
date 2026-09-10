// Intended function: Track aggregate aquatic populations, oxygen, contamination, harvesting, and habitat health.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ecology {

struct AquaticEcologyCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct AquaticEcologyState {
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

struct AquaticEcologyEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class AquaticEcologyService {
public:
    bool apply(const AquaticEcologyCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const AquaticEcologyState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<AquaticEcologyState> ordered() const;
    std::vector<AquaticEcologyEvent> drainEvents();
    void clear();

private:
    AquaticEcologyState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<AquaticEcologyState> states_;
    std::vector<AquaticEcologyEvent> events_;
};

} // namespace elysium::ecology

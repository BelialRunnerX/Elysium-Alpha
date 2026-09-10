// Intended function: Track doors, airlocks, breaches, and room links for coarse audio occlusion/propagation.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::audio {

struct AcousticPortalSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct AcousticPortalSystemState {
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

struct AcousticPortalSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class AcousticPortalSystemService {
public:
    bool apply(const AcousticPortalSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const AcousticPortalSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<AcousticPortalSystemState> ordered() const;
    std::vector<AcousticPortalSystemEvent> drainEvents();
    void clear();

private:
    AcousticPortalSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<AcousticPortalSystemState> states_;
    std::vector<AcousticPortalSystemEvent> events_;
};

} // namespace elysium::audio

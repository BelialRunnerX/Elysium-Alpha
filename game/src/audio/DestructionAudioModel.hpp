// Intended function: Aggregate microvoxel fracture and structural collapse into bounded destruction audio events.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::audio {

struct DestructionAudioModelCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DestructionAudioModelState {
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

struct DestructionAudioModelEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DestructionAudioModelService {
public:
    bool apply(const DestructionAudioModelCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DestructionAudioModelState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DestructionAudioModelState> ordered() const;
    std::vector<DestructionAudioModelEvent> drainEvents();
    void clear();

private:
    DestructionAudioModelState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DestructionAudioModelState> states_;
    std::vector<DestructionAudioModelEvent> events_;
};

} // namespace elysium::audio

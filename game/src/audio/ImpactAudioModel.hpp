// Intended function: Classify impacts by material, energy, penetration, environment, and damage result.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::audio {

struct ImpactAudioModelCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ImpactAudioModelState {
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

struct ImpactAudioModelEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ImpactAudioModelService {
public:
    bool apply(const ImpactAudioModelCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ImpactAudioModelState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ImpactAudioModelState> ordered() const;
    std::vector<ImpactAudioModelEvent> drainEvents();
    void clear();

private:
    ImpactAudioModelState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ImpactAudioModelState> states_;
    std::vector<ImpactAudioModelEvent> events_;
};

} // namespace elysium::audio

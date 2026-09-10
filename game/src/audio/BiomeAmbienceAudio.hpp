// Intended function: Select layered biome ambience from weather, time, ecology, water, and hazard context.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::audio {

struct BiomeAmbienceAudioCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct BiomeAmbienceAudioState {
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

struct BiomeAmbienceAudioEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class BiomeAmbienceAudioService {
public:
    bool apply(const BiomeAmbienceAudioCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const BiomeAmbienceAudioState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<BiomeAmbienceAudioState> ordered() const;
    std::vector<BiomeAmbienceAudioEvent> drainEvents();
    void clear();

private:
    BiomeAmbienceAudioState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<BiomeAmbienceAudioState> states_;
    std::vector<BiomeAmbienceAudioEvent> events_;
};

} // namespace elysium::audio

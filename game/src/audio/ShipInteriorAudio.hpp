// Intended function: Project ship propulsion, hull stress, atmosphere, alarms, machinery, and combat into audio cues.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::audio {

struct ShipInteriorAudioCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ShipInteriorAudioState {
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

struct ShipInteriorAudioEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ShipInteriorAudioService {
public:
    bool apply(const ShipInteriorAudioCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ShipInteriorAudioState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ShipInteriorAudioState> ordered() const;
    std::vector<ShipInteriorAudioEvent> drainEvents();
    void clear();

private:
    ShipInteriorAudioState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ShipInteriorAudioState> states_;
    std::vector<ShipInteriorAudioEvent> events_;
};

} // namespace elysium::audio

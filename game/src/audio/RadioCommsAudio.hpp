// Intended function: Route squad, fleet, colony, emergency, and mission communications through channel policy.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::audio {

struct RadioCommsAudioCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct RadioCommsAudioState {
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

struct RadioCommsAudioEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class RadioCommsAudioService {
public:
    bool apply(const RadioCommsAudioCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const RadioCommsAudioState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<RadioCommsAudioState> ordered() const;
    std::vector<RadioCommsAudioEvent> drainEvents();
    void clear();

private:
    RadioCommsAudioState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<RadioCommsAudioState> states_;
    std::vector<RadioCommsAudioEvent> events_;
};

} // namespace elysium::audio

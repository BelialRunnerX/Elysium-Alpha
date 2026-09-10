// Intended function: Aggregate population, industry, transit, alerts, and district activity into ambience layers.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::audio {

struct SettlementAmbienceAudioCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct SettlementAmbienceAudioState {
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

struct SettlementAmbienceAudioEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class SettlementAmbienceAudioService {
public:
    bool apply(const SettlementAmbienceAudioCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const SettlementAmbienceAudioState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<SettlementAmbienceAudioState> ordered() const;
    std::vector<SettlementAmbienceAudioEvent> drainEvents();
    void clear();

private:
    SettlementAmbienceAudioState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<SettlementAmbienceAudioState> states_;
    std::vector<SettlementAmbienceAudioEvent> events_;
};

} // namespace elysium::audio

// Intended function: Track exploration, threat, combat, discovery, Empire, Rift, and recovery music intensity.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::audio {

struct DynamicMusicStateCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DynamicMusicStateState {
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

struct DynamicMusicStateEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DynamicMusicStateService {
public:
    bool apply(const DynamicMusicStateCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DynamicMusicStateState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DynamicMusicStateState> ordered() const;
    std::vector<DynamicMusicStateEvent> drainEvents();
    void clear();

private:
    DynamicMusicStateState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DynamicMusicStateState> states_;
    std::vector<DynamicMusicStateEvent> events_;
};

} // namespace elysium::audio

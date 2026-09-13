// Intended function: Evaluate gravity-assist opportunities for fuel/time tradeoffs between orbital bodies.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct GravitationalAssistCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct GravitationalAssistState {
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

struct GravitationalAssistEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class GravitationalAssistService {
public:
    bool apply(const GravitationalAssistCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const GravitationalAssistState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<GravitationalAssistState> ordered() const;
    std::vector<GravitationalAssistEvent> drainEvents();
    void clear();

private:
    GravitationalAssistState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<GravitationalAssistState> states_;
    std::vector<GravitationalAssistEvent> events_;
};

} // namespace elysium::stellar

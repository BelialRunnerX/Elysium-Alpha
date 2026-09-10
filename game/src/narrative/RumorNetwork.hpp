// Intended function: Track rumor origin, credibility, propagation, distortion, and discovery across social links.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::narrative {

struct RumorNetworkCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct RumorNetworkState {
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

struct RumorNetworkEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class RumorNetworkService {
public:
    bool apply(const RumorNetworkCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const RumorNetworkState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<RumorNetworkState> ordered() const;
    std::vector<RumorNetworkEvent> drainEvents();
    void clear();

private:
    RumorNetworkState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<RumorNetworkState> states_;
    std::vector<RumorNetworkEvent> events_;
};

} // namespace elysium::narrative

// Intended function: Route empties, waste, damaged goods, reusable packaging, and salvage back through logistics.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::logistics {

struct ReturnLogisticsCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ReturnLogisticsState {
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

struct ReturnLogisticsEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ReturnLogisticsService {
public:
    bool apply(const ReturnLogisticsCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ReturnLogisticsState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ReturnLogisticsState> ordered() const;
    std::vector<ReturnLogisticsEvent> drainEvents();
    void clear();

private:
    ReturnLogisticsState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ReturnLogisticsState> states_;
    std::vector<ReturnLogisticsEvent> events_;
};

} // namespace elysium::logistics

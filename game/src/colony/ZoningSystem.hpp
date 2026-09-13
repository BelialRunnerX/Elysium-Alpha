// Intended function: Track residential, industrial, agricultural, civic, military, and protected-zone policy.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::colony {

struct ZoningSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ZoningSystemState {
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

struct ZoningSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ZoningSystemService {
public:
    bool apply(const ZoningSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ZoningSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ZoningSystemState> ordered() const;
    std::vector<ZoningSystemEvent> drainEvents();
    void clear();

private:
    ZoningSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ZoningSystemState> states_;
    std::vector<ZoningSystemEvent> events_;
};

} // namespace elysium::colony

// Intended function: Aggregate stock capacity and routing hints across linked warehouses without global item scans.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::logistics {

struct WarehouseNetworkCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct WarehouseNetworkState {
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

struct WarehouseNetworkEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class WarehouseNetworkService {
public:
    bool apply(const WarehouseNetworkCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const WarehouseNetworkState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<WarehouseNetworkState> ordered() const;
    std::vector<WarehouseNetworkEvent> drainEvents();
    void clear();

private:
    WarehouseNetworkState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<WarehouseNetworkState> states_;
    std::vector<WarehouseNetworkEvent> events_;
};

} // namespace elysium::logistics

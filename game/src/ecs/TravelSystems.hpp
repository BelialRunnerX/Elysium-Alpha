// Intended function: Stage ECS vehicle/ship seat, pilot, docking, warp, route, and promotion/demotion commands.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct TravelCommand {
    std::uint64_t commandId{};
    std::uint64_t actorId{};
    std::uint64_t vehicleId{};
    std::uint64_t routeId{};
    std::uint64_t kind{};
    std::uint64_t sequence{};
};
class TravelCommandCollection {
public:
 bool store(TravelCommand value); bool erase(std::uint64_t id); [[nodiscard]] const TravelCommand* find(std::uint64_t id) const; [[nodiscard]] const std::vector<TravelCommand>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const TravelCommand& v) noexcept; std::vector<TravelCommand> rows_;
};
}

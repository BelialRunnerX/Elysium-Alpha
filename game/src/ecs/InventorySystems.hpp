// Intended function: Stage ECS inventory/container/equipment/reservation commands around stable item and owner identity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct InventoryCommand {
    std::uint64_t commandId{};
    std::uint64_t actorId{};
    std::uint64_t assetId{};
    std::uint64_t containerId{};
    std::uint64_t units{};
    std::uint64_t sequence{};
};
class InventoryCommandCollection {
public:
 bool store(InventoryCommand value); bool erase(std::uint64_t id); [[nodiscard]] const InventoryCommand* find(std::uint64_t id) const; [[nodiscard]] const std::vector<InventoryCommand>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const InventoryCommand& v) noexcept; std::vector<InventoryCommand> rows_;
};
}

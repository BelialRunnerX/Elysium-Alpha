#pragma once
#include <cstdint>
#include <vector>
namespace elysium::rift {
struct RiftCoord{std::uint8_t x{},y{};bool operator==(const RiftCoord&)const=default;};
struct RiftRoom{std::uint64_t stableRoomId{};RiftCoord coord{};std::uint32_t depth{};bool entrance{},boss{},loot{};bool operator==(const RiftRoom&)const=default;};
struct RiftDoor{std::uint64_t roomA{},roomB{};bool operator==(const RiftDoor&)const=default;};
struct RiftTopology{std::uint64_t riftStableId{},seed{};std::vector<RiftRoom>rooms;std::vector<RiftDoor>doors;bool operator==(const RiftTopology&)const=default;};
struct RiftTopologyConfig{std::uint32_t targetRooms{12};double newestCellBias{0.72};double lootDepthFraction{0.66};};
class RiftTopologyGenerator{public:[[nodiscard]]RiftTopology generate(std::uint64_t riftStableId,std::uint64_t seed,const RiftTopologyConfig& config={},std::uint32_t requestedWorkerCount=1)const;[[nodiscard]]bool validate(const RiftTopology&)const;};
} // namespace elysium::rift

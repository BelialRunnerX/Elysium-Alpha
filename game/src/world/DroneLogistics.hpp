// Intended function: bounded stable-ID drone hauling scheduler over explicit ports, avoiding planet-global inventory scans.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
struct DronePort{std::uint64_t portId{},ownerId{},address{};std::uint32_t capabilityMask{~0u};std::uint16_t queueLimit{8};};
struct DroneUnit{std::uint64_t droneId{};std::uint32_t capabilityMask{~0u},capacity{16};bool busy{};std::uint64_t homePort{};};
struct DroneTransferOrder{std::uint64_t orderId{},sourcePort{},targetPort{};std::uint32_t itemId{},quantity{};std::uint8_t priority{};};
struct DroneMission{std::uint64_t orderId{},droneId{},sourcePort{},targetPort{};std::uint32_t itemId{},quantity{};};
class DroneLogisticsScheduler{public:void upsertPort(DronePort p);void upsertDrone(DroneUnit d);bool submit(DroneTransferOrder o);std::vector<DroneMission>plan(std::size_t maxMissions);void complete(std::uint64_t droneId);private:std::unordered_map<std::uint64_t,DronePort>ports_;std::unordered_map<std::uint64_t,DroneUnit>drones_;std::vector<DroneTransferOrder>orders_;};
}

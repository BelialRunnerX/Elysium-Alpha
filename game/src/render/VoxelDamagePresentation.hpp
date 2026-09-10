// Intended function: presentation-only conversion of authoritative voxel damage into bounded debris, scorch and impact VFX descriptors.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium{
struct VoxelDamageVisualEvent{std::uint64_t eventId{},address{};float x{},y{},z{},severity{};std::uint32_t materialId{};};
struct DebrisInstance{float x{},y{},z{},vx{},vy{},vz{},size{},lifetime{};std::uint32_t materialId{};};
struct ScorchInstance{float x{},y{},z{},radius{},opacity{};std::uint64_t stableKey{};};
struct VoxelDamagePresentation{std::vector<DebrisInstance>debris;std::vector<ScorchInstance>scorches;};
VoxelDamagePresentation buildVoxelDamagePresentation(const VoxelDamageVisualEvent&event,std::uint32_t debrisBudget=32);
}

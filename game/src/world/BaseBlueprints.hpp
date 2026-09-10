// Intended function: reusable stable blueprint descriptions that expand into deterministic batch placement intents rather than mutating voxels directly.
#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace elysium{
struct BlueprintVoxel{std::int16_t x{},y{},z{};std::uint32_t blockId{};std::uint8_t shape{};};
struct BlueprintObject{std::int16_t x{},y{},z{};std::uint32_t objectType{};std::uint64_t localStableKey{};};
struct BaseBlueprint{std::uint64_t blueprintId{};std::string name;std::vector<BlueprintVoxel>voxels;std::vector<BlueprintObject>objects;};
struct BlueprintPlacementIntent{std::uint64_t batchId{},blueprintId{},anchorAddress{};std::int16_t x{},y{},z{};std::uint32_t blockId{},objectType{};std::uint64_t stableKey{};};
bool validateBlueprint(const BaseBlueprint&bp,std::size_t maxVoxels=65536,std::size_t maxObjects=4096);
std::vector<BlueprintPlacementIntent>expandBlueprint(const BaseBlueprint&bp,std::uint64_t anchorAddress,std::uint8_t quarterTurns,std::uint64_t placementSeed);
}

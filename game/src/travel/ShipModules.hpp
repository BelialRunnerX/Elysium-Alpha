// Intended function: modular ship loadout evaluator for mass, power, heat, cargo, shields, thrust, sensors and warp-range progression.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium{
enum class ShipModuleKind:std::uint8_t{Reactor,Engine,WarpDrive,Shield,Weapon,Cargo,Sensor,LifeSupport,Utility,Armor};
struct ShipModuleDefinition{std::uint32_t moduleId{};ShipModuleKind kind{};float mass{},powerDraw{},powerOutput{},heat{},thrust{},warpRating{},shield{},cargo{},sensor{},integrity{};};
struct InstalledShipModule{std::uint64_t stableId{};std::uint32_t moduleId{};float condition{1};bool enabled{true};};
struct ShipPerformance{float mass{},powerMargin{},heat{},thrust{},warpRangeLy{},shield{},cargo{},sensor{},integrity{};bool powerStarved{},overheated{};};
ShipPerformance evaluateShipModules(const std::vector<ShipModuleDefinition>&defs,const std::vector<InstalledShipModule>&installed,float hullMass,float fuelCapacity);
}

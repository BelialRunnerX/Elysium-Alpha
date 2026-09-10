// Intended function: deterministic flora archetype generation and bounded local growth state for destructible, harvestable solarpunk ecosystems.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium{
enum class FloraForm:std::uint8_t{Grass,Shrub,Tree,Vine,Fungus,CrystalPlant,FloatingMoss,Coral};
struct FloraArchetype{std::uint64_t speciesId{};FloraForm form{};float minTemp{},maxTemp{},minMoisture{},maxMoisture{},growthRate{},maxScale{},harvestYield{};std::uint32_t materialItemId{};bool edible{},medicinal{},hazardous{};};
struct FloraPatch{std::uint64_t patchId{},speciesId{},address{};float biomass{},nutrient{},water{};std::uint64_t lastTick{};};
FloraArchetype generateFloraArchetype(std::uint64_t planetSeed,std::uint32_t ordinal);
void advanceFloraPatch(FloraPatch&patch,const FloraArchetype&species,float temperature,float moisture,float light,float dt);
}

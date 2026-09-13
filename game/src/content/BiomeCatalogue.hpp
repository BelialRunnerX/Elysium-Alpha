// Intended function: data-oriented biome catalogue connecting climate envelopes to terrain, flora, fauna, hazards and visual presentation tags.
#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace elysium{
struct BiomeDefinition{std::uint32_t biomeId{};std::string name;float minTemp{},maxTemp{},minMoisture{},maxMoisture{},minElevation{},maxElevation{};std::uint32_t surfaceBlock{},subsurfaceBlock{},hazardMask{},visualTagMask{};float floraDensity{},faunaDensity{},structureChance{};};
class BiomeCatalogue{public:bool add(BiomeDefinition d);const BiomeDefinition*find(std::uint32_t id)const;const BiomeDefinition*select(float temperature,float moisture,float elevation)const;const std::vector<BiomeDefinition>&all()const{return defs_;}private:std::vector<BiomeDefinition>defs_;};
}

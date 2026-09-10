#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::world {using ContentId=std::uint64_t;struct GeologicalLayer{ContentId id{},materialFamily{};double minDepth{},maxDepth{},permeability{},porosity{},aquiferChance{},volatileChance{},structuralTendency{1.0};std::vector<std::pair<ContentId,double>>oreHostModifiers;};struct GeologySample{ContentId layer{};double aquifer{},volatilePocket{},structuralTendency{},cavernSignal{};};class GeologyRuntime{public:explicit GeologyRuntime(std::uint64_t seed):seed_(seed){}bool addLayer(GeologicalLayer,std::string&);std::optional<GeologySample>sample(std::uint64_t column,double depth)const;private:double noise(std::uint64_t,std::uint64_t)const;std::uint64_t seed_;std::vector<GeologicalLayer>layers_;};}
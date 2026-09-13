#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>
namespace elysium::content {
using ContentId=std::uint64_t;
enum class SupportClass:std::uint8_t{None,Weak,Normal,Structural,Reinforced};
struct MaterialDefaults{double hardness{},blastResistance{},thermalConductivity{},corrosionResistance{},friction{},permeability{};bool seal{};};
struct BlockDef{ContentId id{},family{},materialClass{},dropTable{},renderMaterial{},stateSchema{};double hardness{-1},blastResistance{-1},thermalConductivity{-1},corrosionResistance{-1},friction{-1},permeability{-1};std::uint32_t harvestTier{};SupportClass support{SupportClass::Normal};bool seal{};bool sealOverride{};bool microRefinable{};};
struct ResolvedBlock{ContentId id{},family{},materialClass{},dropTable{},renderMaterial{},stateSchema{};double hardness{},blastResistance{},thermalConductivity{},corrosionResistance{},friction{},permeability{};std::uint32_t harvestTier{};SupportClass support{};bool seal{},microRefinable{};};
struct BlockValidation{ContentId id{};std::string code;};
class BlockSchemaRegistry{public:bool addMaterial(ContentId,MaterialDefaults);bool add(BlockDef);std::optional<ResolvedBlock> resolve(ContentId)const;std::vector<BlockValidation> validate()const;bool freeze();bool frozen()const{return frozen_;}const std::map<ContentId,BlockDef>&definitions()const{return blocks_;}private:std::map<ContentId,MaterialDefaults>materials_;std::map<ContentId,BlockDef>blocks_;bool frozen_{};};
}

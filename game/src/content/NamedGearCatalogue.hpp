#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::content { using ContentId=std::uint64_t; enum class GearSlot:std::uint8_t{Weapon,Head,Chest,Hands,Legs,Feet,Accessory}; enum class Rarity:std::uint8_t{Rare,Epic,Legendary}; struct NamedGearRow{ContentId id{};GearSlot slot{};Rarity rarity{};std::uint16_t tier{},levelGate{};ContentId element{},profile{};std::vector<ContentId> passiveRefs,setBonusRefs,acquisitionRefs,socketTags;ContentId setId{},presentation{};}; class NamedGearCatalogue{public:bool add(NamedGearRow,std::string&);bool validate(std::string&)const;std::uint32_t activeSetBonuses(ContentId setId,const std::vector<ContentId>& equipped)const;std::optional<NamedGearRow> get(ContentId)const;private:std::unordered_map<ContentId,NamedGearRow> rows_;}; }
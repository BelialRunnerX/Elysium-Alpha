#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::content { using ContentId=std::uint64_t; enum class TrinketKind:std::uint8_t{Found,Crafted}; struct TrinketRow{ContentId id{};TrinketKind kind{};std::uint16_t minLevel{},tier{};ContentId benefitPassive{},drawbackPassive{};std::vector<ContentId> acquisitionRefs;bool ascendable{false};bool uniqueEquip{false};}; class TrinketCatalogue{public:bool add(TrinketRow,std::string&);bool validatePassiveRefs(const std::unordered_map<ContentId,bool>&,std::string&)const;bool canEquip(ContentId,std::uint16_t,const std::vector<ContentId>&,std::string&)const;private:std::unordered_map<ContentId,TrinketRow> rows_;}; }
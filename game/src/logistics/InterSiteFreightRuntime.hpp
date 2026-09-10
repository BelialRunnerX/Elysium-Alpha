#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::logistics {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class FreightStage:std::uint8_t{Planned,Loading,InTransit,Arrived,Transferred,Failed};struct FreightManifest{StableId id{},origin{},destination{},carrier{},crew{};std::vector<std::pair<ContentId,std::uint64_t>>cargo;double capacity{},fuelCost{},risk{};FreightStage stage{FreightStage::Planned};std::uint64_t departure{},arrival{};};class InterSiteFreightRuntime{public:bool add(FreightManifest,std::string&);bool advanceStrategic(StableId,std::uint64_t tick,std::string&);bool settle(StableId,std::string&);std::optional<FreightManifest>get(StableId)const;private:std::unordered_map<StableId,FreightManifest>rows_;};}
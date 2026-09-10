#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class Routine:std::uint8_t{Reserve,Training,Patrol,Guard,Ready,Siege,Expedition,EvacuationSupport};struct MemberReadiness{StableId member{};double health{1},training{},equipment{},ammo{},access{1};};struct Squad{StableId id{},commander{},barracks{};std::vector<MemberReadiness>members;Routine routine{Routine::Reserve};ContentId equipmentPolicy{};};struct ReadinessSummary{double score{};std::vector<std::string>blockers;};class MilitaryReadinessRuntime{public:bool upsert(Squad);ReadinessSummary readiness(StableId)const;private:std::unordered_map<StableId,Squad>squads_;};}
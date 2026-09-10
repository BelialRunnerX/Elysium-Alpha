// Intended function: compact strategic civilization goals/actions for remote systems, preserving named factions while avoiding active-entity simulation galaxy-wide.
#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace elysium{
enum class StrategicGoal:std::uint8_t{Survive,Expand,Trade,Research,Fortify,Proselytize,Raiding,ImperialCompliance,Liberation,Explore};
enum class StrategicActionKind:std::uint8_t{FoundOutpost,SendCaravan,ResearchProject,BuildDefense,Scout,Raid,Negotiate,Evacuate,Claim,Colonize};
struct CivilizationState{std::uint64_t factionId{};float wealth{},population{},military{},technology{},stability{},imperialStanding{};std::vector<StrategicGoal>goals;};
struct StrategicAction{std::uint64_t actionId{},factionId{},targetId{};StrategicActionKind kind{};float budget{},priority{};};
class CivilizationStrategy{public:void upsert(CivilizationState state);std::vector<StrategicAction>plan(std::uint64_t tick,std::size_t maxActions)const;private:std::unordered_map<std::uint64_t,CivilizationState>states_;};
}

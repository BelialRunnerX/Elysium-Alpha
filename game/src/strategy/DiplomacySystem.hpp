// Intended function: bilateral faction standing, treaties, incidents and war/peace transitions keyed only by durable faction IDs.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
enum class TreatyKind:std::uint8_t{Trade,NonAggression,Defense,Research,Transit,Embargo,War,Peace};
struct DiplomaticRelation{std::uint64_t factionA{},factionB{};float standing{};std::vector<TreatyKind>treaties;std::uint64_t lastIncidentTick{};};
struct DiplomaticIncident{std::uint64_t incidentId{},actorFaction{},targetFaction{};float standingDelta{};std::uint32_t reason{};std::uint64_t tick{};};
class DiplomacySystem{public:void setStanding(std::uint64_t a,std::uint64_t b,float value);void addTreaty(std::uint64_t a,std::uint64_t b,TreatyKind treaty);void removeTreaty(std::uint64_t a,std::uint64_t b,TreatyKind treaty);void applyIncident(DiplomaticIncident incident);std::optional<DiplomaticRelation>relation(std::uint64_t a,std::uint64_t b)const;std::vector<DiplomaticIncident>drainIncidents();private:static std::uint64_t key(std::uint64_t a,std::uint64_t b);std::unordered_map<std::uint64_t,DiplomaticRelation>relations_;std::vector<DiplomaticIncident>incidents_;};
}

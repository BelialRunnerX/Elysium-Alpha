#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::history {using StableId=std::uint64_t;enum class HistoryEventType:std::uint16_t{LifeBond,SiteFounded,SiteDestroyed,OfficeChanged,Conflict,Artifact,Knowledge,Justice,Threat,EmpireAction,PlayerSignificant};struct Civilization{StableId id{};std::uint64_t population{};std::vector<StableId> sites,leaders,relations;};struct HistoryEvent{StableId id{};HistoryEventType type{};std::uint64_t epoch{};StableId subject{},object{},site{};bool significant{false};};class WorldHistory{public:explicit WorldHistory(std::uint64_t seed,std::uint32_t version):seed_(seed),version_(version){}StableId deterministicId(std::uint64_t domain,std::uint64_t address)const;bool addCivilization(Civilization,std::string&);bool append(HistoryEvent,std::string&);const std::vector<HistoryEvent>&events()const{return events_;}std::vector<HistoryEvent>chronicleCandidates()const;std::uint64_t stableHash()const;private:std::uint64_t seed_;std::uint32_t version_;std::unordered_map<StableId,Civilization>civs_;std::vector<HistoryEvent>events_;};}
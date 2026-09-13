#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class SiteState:std::uint8_t{GeneratedUnknown,ActiveStrategic,ActiveDetailed,BesiegedDisrupted,Abandoned,RuinedContaminated,OccupiedReused,Reclaimed};struct SiteLifecycle{StableId id{},baselineKit{},occupantFaction{};SiteState state{SiteState::GeneratedUnknown};std::uint64_t spatialDeltaRevision{},historyRevision{};ContentId generatorFingerprint{};};class SiteLifecycleRuntime{public:bool add(SiteLifecycle,std::string&);bool transition(StableId,SiteState,std::string&);std::optional<SiteLifecycle>get(StableId)const;private:std::unordered_map<StableId,SiteLifecycle>rows_;};}
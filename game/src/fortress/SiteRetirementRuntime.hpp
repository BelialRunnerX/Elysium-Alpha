#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class RepresentationState:std::uint8_t{Detailed,Retiring,Strategic,Reclaiming};struct SiteRepresentation{StableId site{};RepresentationState state{RepresentationState::Detailed};ContentId generatorFingerprint{};std::uint64_t entityRevision{},spatialDeltaRevision{},historyRevision{};std::vector<StableId>persistentEntities;std::uint64_t aggregatePopulation{};};class SiteRetirementRuntime{public:bool add(SiteRepresentation,std::string&);bool retire(StableId,bool localCommitsFrozen,std::string&);bool strategicAdvance(StableId,std::uint64_t populationDelta,std::string&);bool reclaim(StableId,ContentId currentFingerprint,bool migrationAvailable,std::string&);std::optional<SiteRepresentation>get(StableId)const;private:std::unordered_map<StableId,SiteRepresentation>rows_;};}
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::history {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct ProvenanceEvent{StableId id{};ContentId type{};StableId from{},to{},site{};std::uint64_t time{};};struct Artifact{StableId id{},item{},maker{},creationSite{},owner{};ContentId definition{},material{},culture{};std::uint64_t creationTime{};std::vector<ProvenanceEvent>provenance;};class ArtifactRuntime{public:bool promote(Artifact,std::string&);bool transfer(StableId,ProvenanceEvent,std::string&);std::optional<Artifact>get(StableId)const;private:std::unordered_map<StableId,Artifact>rows_;};}
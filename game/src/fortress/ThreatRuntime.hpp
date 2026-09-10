#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct ThreatFamily{ContentId id{};std::vector<ContentId>causeTags,downstreamPipelines,grammarTags;};struct DefenseNode{StableId id{};double exposure{},value{},knownConfidence{};};struct SiegePlan{StableId id{};ContentId family{};std::vector<StableId>approach,targets;double score{};};class ThreatRuntime{public:bool addFamily(ThreatFamily,std::string&);std::optional<SiegePlan>plan(ContentId,std::uint64_t seed,const std::vector<DefenseNode>&known,std::string&)const;private:std::unordered_map<ContentId,ThreatFamily>families_;};}
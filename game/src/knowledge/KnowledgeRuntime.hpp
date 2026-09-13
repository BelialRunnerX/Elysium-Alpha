#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::knowledge {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class KnowledgeKind:std::uint8_t{Fact,Technique,Theory,Map,HistoricalAccount,CulturalWork,ForbiddenProtocol};struct KnowledgeObject{ContentId id{};std::uint32_t version{1};KnowledgeKind kind{};double confidence{};StableId provenance{},physicalMedia{};double legalRisk{};};struct Holding{StableId holder{};ContentId knowledge{};double confidence{};bool accessible{true};};class KnowledgeRuntime{public:bool add(KnowledgeObject,std::string&);bool grant(Holding,std::string&);bool transfer(StableId from,StableId to,ContentId,double loss,std::string&);std::vector<Holding>holdings(ContentId)const;private:std::unordered_map<ContentId,KnowledgeObject>defs_;std::vector<Holding>hold_;};}
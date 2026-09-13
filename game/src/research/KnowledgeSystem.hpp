// Intended function: persistent knowledge graph for facts, techniques, theories, maps and forbidden protocols with provenance/confidence and unlock dependencies.
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium{
enum class KnowledgeKind:std::uint8_t{Fact,Technique,Theory,Map,HistoricalAccount,CulturalWork,ForbiddenProtocol};
struct KnowledgeRecord{std::uint64_t knowledgeId{};KnowledgeKind kind{};std::string title;float confidence{};std::uint64_t sourceStableId{},authorStableId{},locationId{};std::vector<std::uint64_t>prerequisites;std::uint32_t unlockTagMask{};bool restricted{};};
class KnowledgeSystem{public:bool add(KnowledgeRecord record);bool improveConfidence(std::uint64_t id,float delta,std::uint64_t source);std::optional<KnowledgeRecord>find(std::uint64_t id)const;std::vector<KnowledgeRecord>availableTechniques()const;private:std::unordered_map<std::uint64_t,KnowledgeRecord>records_;};
}

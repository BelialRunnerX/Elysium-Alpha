#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <vector>
namespace elysium::exploration {
enum class ResearchDomain:std::uint8_t{Geology,Xenobiology,Environmental,ImperialSystems,RiftScience,Engineering};
enum class EvidenceKind:std::uint8_t{Scan,OreCore,Cave,Flora,Fauna,Hazard,ImperialInfrastructure,Anomaly,Machine,Ruin,Derelict,Other};
enum class ResearchGrantKind:std::uint8_t{Blueprint,ScannerInterpretation,AutomationRecipe,ExpeditionCapability,InformationTool};
enum class ResearchProcessingCapability:std::uint32_t{ResearchConsole=1u<<0,MapRoom=1u<<1};
constexpr std::uint32_t processingCapability(ResearchProcessingCapability c)noexcept{return static_cast<std::uint32_t>(c);}
struct ResearchEvidenceEvent{std::uint64_t stableEvidenceId{},sourceStableId{};ResearchDomain domain{};EvidenceKind kind{};std::uint64_t points{};double confidence{1.0};std::vector<std::uint64_t>tagContentIds;};
struct ResearchUnlockDefinition{std::uint64_t unlockContentId{};ResearchDomain domain{};std::uint64_t pointsRequired{};double minimumConfidence{};std::vector<std::uint64_t>requiredTagContentIds;std::uint32_t requiredProcessingMask{};ResearchGrantKind grantKind{ResearchGrantKind::Blueprint};};
struct ResearchGrant{std::uint64_t unlockContentId{};ResearchGrantKind kind{};ResearchDomain domain{};std::uint64_t qualifyingPoints{};};
struct ResearchEvaluation{std::vector<ResearchGrant>newGrants;std::vector<std::uint64_t>blockedUnlockContentIds;};
struct ResearchSnapshot{std::vector<ResearchUnlockDefinition>definitions;std::vector<ResearchEvidenceEvent>evidence;std::vector<std::uint64_t>grantedContentIds;};
class ResearchEvidenceRuntime{public:
 bool publish(ResearchUnlockDefinition);bool addEvidence(ResearchEvidenceEvent);[[nodiscard]]bool hasEvidence(std::uint64_t)const;[[nodiscard]]bool granted(std::uint64_t)const;[[nodiscard]]std::uint64_t fieldPoints(ResearchDomain)const;ResearchEvaluation evaluate(std::uint32_t processingMask);[[nodiscard]]ResearchSnapshot snapshot()const;bool restore(const ResearchSnapshot&);
private:bool qualifies(const ResearchUnlockDefinition&,std::uint64_t*points=nullptr)const;std::map<std::uint64_t,ResearchUnlockDefinition>definitions_;std::map<std::uint64_t,ResearchEvidenceEvent>evidence_;std::set<std::uint64_t>granted_;
};
} // namespace elysium::exploration

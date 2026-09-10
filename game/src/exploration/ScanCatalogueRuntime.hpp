#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::exploration {
enum class ScanKind:std::uint8_t{System,Planet,Flora,Fauna,MineralVein,Structure,Anomaly};
struct ScanDefinition{std::uint64_t contentId{};ScanKind kind{};std::uint64_t requiredToolMask{},requiredKnowledgeMask{};double baseRangeMeters{},energyCost{};std::uint64_t chargeCost{};bool requiresLineOfSight{},spendOnBlocked{};std::uint64_t knownFieldMask{};};
struct ScannerAccount{std::uint64_t stableId{};double energy{};std::uint64_t charges{},revision{1};};
struct ScanSubject{std::uint64_t stableTargetId{};ScanKind kind{};double distanceMeters{};bool lineOfSight{};std::uint64_t truthVersion{};};
struct ScanModifiers{std::uint64_t toolMask{},knowledgeMask{};double equipmentRangeMultiplier{1.0},relayRangeMultiplier{1.0},interpretationMultiplier{1.0};};
enum class ScanBlock:std::uint8_t{None,MissingDefinition,MissingScanner,InvalidTarget,WrongKind,MissingTool,MissingKnowledge,OutOfRange,NoLineOfSight,InsufficientEnergy,InsufficientCharges,AlreadyCatalogued};
struct CatalogueRecord{std::uint64_t stableTargetId{},scanDefinitionContentId{},scannerStableId{},knownFieldMask{},catalogueRevision{1};ScanKind kind{};double confidence{};};
struct LocalScanMarker{std::uint64_t stableTargetId{};ScanKind kind{};double confidence{};};
struct ScanOutcome{bool accepted{};ScanBlock block{ScanBlock::None};double energySpent{};std::uint64_t chargesSpent{};double effectiveRangeMeters{};CatalogueRecord record{};LocalScanMarker marker{};};
struct ScanSnapshot{std::vector<ScanDefinition>definitions;std::vector<ScannerAccount>scanners;std::vector<CatalogueRecord>catalogue;};
class ScanCatalogueRuntime{public:
 bool publish(ScanDefinition);bool publish(ScannerAccount);[[nodiscard]]const ScannerAccount*scanner(std::uint64_t)const;[[nodiscard]]const CatalogueRecord*known(std::uint64_t)const;ScanOutcome attempt(std::uint64_t definitionContentId,std::uint64_t scannerStableId,const ScanSubject&,const ScanModifiers&);[[nodiscard]]ScanSnapshot snapshot()const;bool restore(const ScanSnapshot&);
private:std::map<std::uint64_t,ScanDefinition>definitions_;std::map<std::uint64_t,ScannerAccount>scanners_;std::map<std::uint64_t,CatalogueRecord>catalogue_;
};
} // namespace elysium::exploration

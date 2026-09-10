#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <vector>
namespace elysium::poi {
enum class PoiModuleRole:std::uint8_t{ApproachApron,SilhouetteMass,EntryThreshold,ServiceCore,LootCache,PowerStub,EncounterAnchor,VerticalConnector,DamageOverlay,FactionDecal};
struct PoiModuleDef{std::uint64_t contentId{};PoiModuleRole role{};bool required{true};double inclusionProbability{1.0};std::uint64_t compatibilityMask{~0ULL};};
struct PoiKitDef{std::uint64_t contentId{},factionGrammarContentId{},locationCompatibilityMask{};std::vector<PoiModuleDef>modules;};
struct PoiPlacementKey{std::uint64_t worldSeed{},worldId{},spatialAddress{},generatorVersion{1};};
struct PoiModulePlacement{std::uint64_t stableObjectId{},moduleContentId{};PoiModuleRole role{};std::uint32_t orientationQuarterTurns{},damagePermille{},variant{};bool operator==(const PoiModulePlacement&)const=default;};
struct PoiKitPersistentState{std::set<std::uint64_t>tombstonedObjectIds;std::map<std::uint64_t,std::uint32_t>addedDamagePermille;};
struct PoiKitPlacement{std::uint64_t kitContentId{},placementStableId{},encounterSeed{};std::uint32_t orientationQuarterTurns{};std::vector<PoiModulePlacement>modules;};
class PoiKitPlacementGenerator{public:bool publish(PoiKitDef);[[nodiscard]]PoiKitPlacement generate(std::uint64_t kitContentId,const PoiPlacementKey& key,const PoiKitPersistentState& persistentState = {},std::uint32_t requestedWorkerCount=1)const;private:std::map<std::uint64_t,PoiKitDef>kits_;};
} // namespace elysium::poi

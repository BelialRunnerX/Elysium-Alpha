// Intended function: deterministic mod manifest/dependency planning and stable namespace ownership without allowing mods to become hidden save authority.
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium{
struct ModDependency{std::string modId;std::uint32_t minVersion{},maxVersion{0xFFFFFFFFu};bool optional{};};
struct ModManifest{std::string modId,name;std::uint32_t version{1};std::vector<ModDependency>dependencies;std::vector<std::string>contentNamespaces;std::uint64_t compatibilityFingerprint{};};
struct ModLoadPlan{bool valid{};std::vector<std::string>orderedModIds;std::vector<std::string>errors,warnings;std::uint64_t combinedFingerprint{};};
ModLoadPlan planModLoad(std::vector<ModManifest> manifests);
enum class ModHook:std::uint8_t{WorldGenerated,ChunkLoaded,EntitySpawned,ItemCrafted,CombatResolved,ContractResolved,HistoryEvent};
struct ModEventEnvelope{ModHook hook{};std::uint64_t subjectId{},locationId{},eventSeed{};std::vector<std::uint64_t>arguments;};
class ModEventQueue{public:void publish(ModEventEnvelope event);std::vector<ModEventEnvelope>drainDeterministic();private:std::vector<ModEventEnvelope>events_;};
}

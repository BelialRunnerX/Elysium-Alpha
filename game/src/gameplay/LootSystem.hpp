// Intended function: Diablo-style deterministic loot generation with stable item identity, tiers, affixes, sockets and provenance-ready roll data.
#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace elysium{
enum class LootTier:std::uint8_t{Common,Uncommon,Rare,Epic,Relic,Artifact};
enum class AffixOp:std::uint8_t{Add,Multiply,Percent,Trigger};
struct LootAffixDefinition{std::uint32_t affixId{};std::string name;AffixOp op{AffixOp::Add};float minValue{},maxValue{};std::uint32_t tagMask{~0u};};
struct LootBaseDefinition{std::uint32_t itemId{};std::uint32_t tagMask{~0u};std::uint32_t weight{1};std::uint8_t maxSockets{3};};
struct LootTable{std::uint32_t tableId{};std::vector<LootBaseDefinition>bases;std::vector<LootAffixDefinition>affixes;};
struct LootRollContext{std::uint64_t worldSeed{},sourceStableId{},eventOrdinal{};std::uint32_t level{1};float magicFind{};};
struct RolledAffix{std::uint32_t affixId{};float value{};};
struct GeneratedLoot{std::uint64_t stableItemId{};std::uint32_t baseItemId{};LootTier tier{LootTier::Common};std::uint8_t sockets{};std::vector<RolledAffix>affixes;std::uint64_t provenanceSeed{};};
GeneratedLoot rollLoot(const LootTable& table,const LootRollContext& context);
}

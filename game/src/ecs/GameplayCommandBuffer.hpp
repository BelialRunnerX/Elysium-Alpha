// Intended function: deterministic cross-system command vocabulary for abilities, effects, projectiles, entities, world edits and inventory transfers.
#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace elysium {

enum class GameplayCommandKind : std::uint8_t { SpawnEntity, DespawnEntity, ActivateAbility, ApplyEffect, SpawnProjectile, Damage, WorldEdit, InventoryTransfer, EmitHistory };
struct SpawnEntityCommand{std::uint64_t stableId{},archetypeId{},locationKey{};};
struct DespawnEntityCommand{std::uint64_t stableId{};};
struct ActivateAbilityCommand{std::uint64_t actorId{},abilityId{},targetId{},targetAddress{};};
struct ApplyEffectCommand{std::uint64_t sourceId{},targetId{},effectId{};float magnitude{},duration{};};
struct SpawnProjectileCommand{std::uint64_t projectileId{},sourceId{},targetId{};float speed{},damage{};};
struct DamageGameplayCommand{std::uint64_t sourceId{},targetId{};float amount{};std::uint32_t damageMask{};};
struct WorldEditGameplayCommand{std::uint64_t address{};std::uint32_t materialId{};std::int16_t delta{};};
struct InventoryTransferGameplayCommand{std::uint64_t fromId{},toId{};std::uint32_t itemId{},quantity{};};
struct EmitHistoryGameplayCommand{std::uint64_t eventId{},subjectId{},locationId{};std::uint32_t eventType{};};
using GameplayCommandPayload=std::variant<SpawnEntityCommand,DespawnEntityCommand,ActivateAbilityCommand,ApplyEffectCommand,SpawnProjectileCommand,DamageGameplayCommand,WorldEditGameplayCommand,InventoryTransferGameplayCommand,EmitHistoryGameplayCommand>;
struct GameplayCommand{GameplayCommandKind kind{};std::uint8_t phase{};std::uint64_t stableKey{},producerStableId{},localOrdinal{};GameplayCommandPayload payload;};
class GameplayCommandBuffer{
public:
 void submit(GameplayCommand command);
 void append(std::vector<GameplayCommand> commands);
 std::vector<GameplayCommand> drainDeterministic();
 std::size_t size()const{return commands_.size();}
 void clear(){commands_.clear();}
private:std::vector<GameplayCommand> commands_;
};
} // namespace elysium

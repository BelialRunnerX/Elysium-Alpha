// Intended function: bounded persistent/timed status-effect stacks with explicit tick outputs for combat, survival and UI consumers.
#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace elysium{
enum class EffectStackRule:std::uint8_t{Refresh,AddDuration,AddMagnitude,Independent,ReplaceStronger};
struct EffectDefinition{std::uint64_t effectId{};EffectStackRule stack{EffectStackRule::Refresh};float tickInterval{1},maxMagnitude{1000};std::uint8_t maxStacks{1};};
struct ActiveEffect{std::uint64_t sourceId{},targetId{},effectId{},instanceId{};float magnitude{},remaining{},untilTick{};};
struct EffectTick{std::uint64_t sourceId{},targetId{},effectId{};float magnitude{};};
class EffectSystem{public:void registerDefinition(EffectDefinition d);bool apply(ActiveEffect e);std::vector<EffectTick>advance(float dt);const std::vector<ActiveEffect>&active()const{return active_;}private:std::unordered_map<std::uint64_t,EffectDefinition>defs_;std::vector<ActiveEffect>active_;std::unordered_map<std::uint64_t,float>accum_;};
}

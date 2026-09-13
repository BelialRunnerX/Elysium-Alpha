#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace elysium::content {
using ContentId=std::uint64_t;
struct EnemyVerb{ContentId actionId{};double weight{},minimumRange{},maximumRange{};};
struct EnemyArchetype{ContentId archetypeId{},bodyPlanId{},factionId{},roleId{},lootTableId{};double healthScale{1},damageScale{1},speedScale{1};std::vector<EnemyVerb>verbs;ContentId transformsInto{};double transformHealthFraction{};};
class BestiaryArchetypeRegistry{public:bool publish(EnemyArchetype a,reason::ReasonStack*r=nullptr);[[nodiscard]]const EnemyArchetype*find(ContentId id)const;private:std::unordered_map<ContentId,EnemyArchetype>defs_;};
} // namespace elysium::content

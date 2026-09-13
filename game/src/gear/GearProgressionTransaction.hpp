#pragma once
#include "rules/CharacterRpg.hpp"
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <optional>
#include <unordered_set>
namespace elysium::gear {
struct ReforgeTransactionRequest{std::uint64_t transactionId{},itemStableId{},entropy{};double reforgeScale{1.0};};
struct ReforgeTransactionPlan{bool accepted{};reason::ReasonStack reasons;rpg::GearItemState resultingItem;};
struct AscensionTransactionRequest{std::uint64_t transactionId{},targetStableId{},counterpartStableId{};};
struct AscensionTransactionPlan{bool accepted{};reason::ReasonStack reasons;rpg::GearItemState resultingTarget;std::optional<rpg::GearItemState> resultingCounterpart;};
class GearProgressionTransactionPlanner{public:[[nodiscard]]ReforgeTransactionPlan planReforge(const ReforgeTransactionRequest&r,const rpg::GearItemState&item,const rpg::GearItemDefinition&def)const;[[nodiscard]]AscensionTransactionPlan planAscension(const AscensionTransactionRequest&r,const rpg::GearItemState&target,const rpg::GearItemState&counterpart,const rpg::GearItemDefinition&def)const;};
class GearTransactionLedger{public:bool commit(std::uint64_t transactionId);[[nodiscard]]bool seen(std::uint64_t transactionId)const;private:std::unordered_set<std::uint64_t>ids_;};
} // namespace elysium::gear

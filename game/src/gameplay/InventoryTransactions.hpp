// Intended function: atomic two-phase inventory transaction records for crafting, trade, hauling, loot and machine transfers.
#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace elysium{
struct InventoryLine{std::uint64_t ownerId{};std::uint32_t itemId{};std::int64_t quantityDelta{};};
enum class InventoryTransactionState:std::uint8_t{Prepared,Committed,RolledBack,Rejected};
struct InventoryTransaction{std::uint64_t transactionId{};InventoryTransactionState state{InventoryTransactionState::Prepared};std::vector<InventoryLine>lines;};
class InventoryTransactionLedger{public:bool prepare(InventoryTransaction t);bool commit(std::uint64_t id,std::unordered_map<std::uint64_t,std::unordered_map<std::uint32_t,std::uint32_t>>&inventories);bool rollback(std::uint64_t id);const InventoryTransaction*find(std::uint64_t id)const;private:std::unordered_map<std::uint64_t,InventoryTransaction>transactions_;};
}

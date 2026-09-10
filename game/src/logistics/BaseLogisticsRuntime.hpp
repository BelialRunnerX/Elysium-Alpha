#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace elysium::logistics {
using StableId=std::uint64_t;using ContentId=std::uint64_t;
struct StorageStack{ContentId itemId{};std::uint64_t count{};};
struct NetworkStorageNode{StableId nodeId{};std::uint64_t baseId{};std::uint64_t networkId{};std::uint64_t capacityUnits{};std::vector<StorageStack> stacks;bool powered{true};};
enum class TransportKind:std::uint8_t{Belt,Sorter,CargoLoader};
struct TransportEdge{StableId edgeId{},sourceNode{},targetNode{};std::uint64_t baseId{};TransportKind kind{TransportKind::Belt};ContentId filterItem{};double unitsPerSecond{};bool powered{true};};
struct TransferRequest{std::uint64_t transactionId{};std::uint64_t baseId{};ContentId itemId{};std::uint64_t count{};StableId sourceNode{},targetNode{};};
struct TransferPlan{bool accepted{};reason::ReasonStack reasons;std::uint64_t count{};};
class BaseLogisticsRuntime{
public:bool upsertStorage(NetworkStorageNode node);bool upsertEdge(TransportEdge edge);[[nodiscard]]const NetworkStorageNode*storage(StableId id)const;[[nodiscard]]TransferPlan plan(const TransferRequest&r)const;bool commit(const TransferRequest&r,const TransferPlan&p);[[nodiscard]]std::vector<NetworkStorageNode>snapshot()const;
private:std::unordered_map<StableId,NetworkStorageNode>nodes_;std::unordered_map<StableId,TransportEdge>edges_;std::unordered_set<std::uint64_t>transactions_;};
} // namespace elysium::logistics

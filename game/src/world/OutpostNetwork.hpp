// Intended function: compact remote representation of multiple outposts and inter-site transfers for strategic simulation outside active shards.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
struct OutpostStock{std::uint32_t itemId{},quantity{},desiredReserve{};};
struct OutpostRecord{std::uint64_t outpostId{},systemId{},siteId{},ownerFactionId{};float powerReliability{1},habitability{1},threat{};std::uint32_t population{};std::vector<OutpostStock>stocks;};
struct OutpostTransfer{std::uint64_t transferId{},fromOutpost{},toOutpost{};std::uint32_t itemId{},quantity{};std::uint64_t departTick{},arrivalTick{};};
class OutpostNetwork{public:bool upsert(OutpostRecord outpost);std::optional<OutpostRecord>find(std::uint64_t id)const;std::vector<OutpostTransfer>planDeficitTransfers(std::uint64_t tick,std::uint32_t maxTransfers,std::uint64_t travelTicks);void applyArrival(const OutpostTransfer&transfer);private:std::unordered_map<std::uint64_t,OutpostRecord>outposts_;std::uint64_t nextTransfer_{1};};
}

#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::items {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class IdentityTier:std::uint8_t{Bulk,Stack,Individual,NamedOwned,Artifact};struct ItemState{StableId id{};ContentId material{};std::uint64_t quantity{};std::uint32_t quality{};double condition{1};IdentityTier tier{IdentityTier::Bulk};StableId owner{},reservation{};bool modified{},evidence{},historySignificant{};};struct OwnershipClaim{StableId item{},owner{};ContentId reason{};std::uint64_t revision{};};class ItemIdentityRuntime{public:bool add(ItemState,std::string&);bool promote(StableId,IdentityTier,std::string&);bool canReaggregate(StableId)const;bool transferOwnership(StableId,StableId,ContentId,std::string&);std::optional<ItemState>get(StableId)const;private:std::unordered_map<StableId,ItemState>items_;std::unordered_map<StableId,OwnershipClaim>claims_;};}
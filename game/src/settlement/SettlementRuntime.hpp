#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::settlement {
enum class SettlementTier:std::uint8_t{Camp,Hamlet,FrontierTown};
enum class SettlementFactionProfile:std::uint8_t{NeutralFrontier,ImperialEnclave,UnswornHaven,PlayerOutpost};
enum class SettlementLodState:std::uint8_t{Detailed,SleepingStrategic};
enum class SettlementService:std::uint32_t{Trade=1u<<0,Rumors=1u<<1,Needs=1u<<2,Fears=1u<<3,Logistics=1u<<4,Power=1u<<5,ShipPad=1u<<6,Market=1u<<7,Clinic=1u<<8,Workshop=1u<<9,Inspection=1u<<10,FileExposure=1u<<11,Salvage=1u<<12,Charts=1u<<13,UnregisteredTrade=1u<<14,Favor=1u<<15,Claim=1u<<16,Beacon=1u<<17,Automation=1u<<18,Siege=1u<<19,Contracts=1u<<20};
constexpr std::uint32_t settlementService(SettlementService s)noexcept{return static_cast<std::uint32_t>(s);}
struct SettlementTierDefinition{std::uint64_t contentId{};SettlementTier tier{};std::uint64_t populationMetadataMin{},populationMetadataMax{};std::uint32_t baseServices{};};
struct SettlementFactionDefinition{std::uint64_t contentId{};SettlementFactionProfile profile{};std::uint32_t addServices{},removeServices{},powerRequiredServices{};};
struct SettlementState{std::uint64_t stableId{},siteStableId{},tierContentId{},factionContentId{},ownerStableId{},revision{1},rumorHookContentId{},needHookContentId{},fearHookContentId{};SettlementLodState lod{SettlementLodState::Detailed};bool powered{};std::uint64_t populationEstimate{};};
enum class SettlementServiceBlock:std::uint8_t{None,MissingSettlement,UnknownTier,UnknownFaction,CapabilityAbsent,MissingPower};
struct SettlementServiceProjection{bool available{};bool requiresWake{};SettlementServiceBlock block{SettlementServiceBlock::None};std::uint64_t settlementStableId{};};
struct SettlementSnapshot{std::vector<SettlementTierDefinition>tiers;std::vector<SettlementFactionDefinition>factions;std::vector<SettlementState>settlements;};
class SettlementRuntime{public:bool publish(SettlementTierDefinition);bool publish(SettlementFactionDefinition);bool create(SettlementState);bool setPower(std::uint64_t,bool);bool setLod(std::uint64_t,SettlementLodState);[[nodiscard]]const SettlementState*find(std::uint64_t)const;[[nodiscard]]std::uint32_t services(std::uint64_t)const;[[nodiscard]]SettlementServiceProjection query(std::uint64_t,SettlementService)const;[[nodiscard]]SettlementSnapshot snapshot()const;bool restore(const SettlementSnapshot&);private:std::map<std::uint64_t,SettlementTierDefinition>tiers_;std::map<std::uint64_t,SettlementFactionDefinition>factions_;std::map<std::uint64_t,SettlementState>settlements_;};
} // namespace elysium::settlement

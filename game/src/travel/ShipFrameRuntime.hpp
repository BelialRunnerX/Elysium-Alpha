#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::travel {
enum class ShipSlotDomain:std::uint8_t{Propulsion,Utility,Science,Defense,Control,Safety,Industry,Faction};
enum class ShipCapability:std::uint32_t{PulseTransit=1u<<0,AtmosphericThruster=1u<<1,VectorControl=1u<<2,DeepScanner=1u<<3,AtmosphereScanner=1u<<4,Docking=1u<<5,Autopilot=1u<<6};
constexpr std::uint32_t capability(ShipCapability c)noexcept{return static_cast<std::uint32_t>(c);}
struct SlotCapacity{ShipSlotDomain domain{};std::uint32_t slots{};};
struct ShipFrameDefinition{std::uint64_t contentId{};std::uint64_t cargoCapacity{},fuelCellCapacity{};double hull{},atmosphericHandling{};std::uint32_t scannerGrade{},baseJumpTier{};std::vector<SlotCapacity>slots;};
struct ShipModuleDefinition{std::uint64_t contentId{};ShipSlotDomain domain{};std::uint32_t slotCost{1},capabilities{};std::uint64_t cargoBonus{},fuelBonus{};double hullBonus{},handlingBonus{};std::uint32_t scannerBonus{},jumpTier{};};
struct InstalledShipModule{std::uint64_t stableItemId{},moduleContentId{};bool enabled{true};};
struct JumpTierDefinition{std::uint32_t tier{};double maximumRangeLy{};std::uint64_t fuelCellsPerJump{};};
struct ShipState{std::uint64_t stableId{},frameContentId{},revision{1},fuelCells{};double hull{};std::vector<InstalledShipModule>modules;};
struct ShipDerivedStats{bool valid{};std::uint64_t cargoCapacity{},fuelCellCapacity{};double maximumHull{},atmosphericHandling{},jumpRangeLy{};std::uint32_t scannerGrade{},jumpTier{},capabilities{};};
enum class ShipBlockReason:std::uint8_t{None,MissingShip,UnknownFrame,UnknownModule,SlotConflict,DuplicateModule,MalformedModule,InsufficientFuel,RangeExceeded,MissingWarpTier,InvalidDistance};
struct ShipCheck{bool allowed{};ShipBlockReason reason{ShipBlockReason::None};std::uint64_t fuelRequired{};double maximumRangeLy{};};
struct ShipSnapshot{std::vector<ShipFrameDefinition>frames;std::vector<ShipModuleDefinition>modules;std::vector<JumpTierDefinition>jumpTiers;std::vector<ShipState>ships;};
class ShipFrameRuntime{public:
 bool publish(ShipFrameDefinition);bool publish(ShipModuleDefinition);bool publish(JumpTierDefinition);bool create(ShipState);bool setModules(std::uint64_t shipId,std::vector<InstalledShipModule>);bool migrateFrame(std::uint64_t shipId,std::uint64_t frameContentId);
 [[nodiscard]]const ShipState*find(std::uint64_t shipId)const;[[nodiscard]]ShipDerivedStats derive(std::uint64_t shipId)const;[[nodiscard]]ShipCheck checkJump(std::uint64_t shipId,double distanceLy)const;[[nodiscard]]ShipSnapshot snapshot()const;bool restore(const ShipSnapshot&);
private:std::map<std::uint64_t,ShipFrameDefinition>frames_;std::map<std::uint64_t,ShipModuleDefinition>modules_;std::map<std::uint32_t,JumpTierDefinition>tiers_;std::map<std::uint64_t,ShipState>ships_;
};
} // namespace elysium::travel

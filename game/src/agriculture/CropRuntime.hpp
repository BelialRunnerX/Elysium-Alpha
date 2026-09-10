#pragma once

#include <cstdint>
#include <map>
#include <vector>

namespace elysium::agriculture {

enum class CultivationCapability:std::uint32_t{TilledSoil=1u<<0,ManualWater=1u<<1,Irrigation=1u<<2,Greenhouse=1u<<3,Hydroponic=1u<<4,ControlledEnvironment=1u<<5};
constexpr std::uint32_t capability(CultivationCapability c) noexcept{return static_cast<std::uint32_t>(c);}
enum class CropStage:std::uint8_t{Seeded,Germinating,Vegetative,Flowering,Mature,Failed};
enum class CropBlocker:std::uint32_t{None=0,Dry=1u<<0,Flooded=1u<<1,LowFertility=1u<<2,InvalidAtmosphere=1u<<3,Temperature=1u<<4,Light=1u<<5,MissingSeal=1u<<6,MissingPollination=1u<<7,MissingCapability=1u<<8};
constexpr std::uint32_t blocker(CropBlocker b) noexcept{return static_cast<std::uint32_t>(b);}

struct CropDefinition{
 std::uint64_t cropContentId{};std::vector<std::uint64_t> stageTicks;
 double minimumLight{},minimumTemperature{},maximumTemperature{1},minimumOxygen{},minimumPressure{},maximumWaterSaturation{1};
 bool requiresPollination{},requiresSealedRoom{};std::uint32_t requiredCapabilities{};
 std::uint64_t waterUnitsPerStep{1},organicUnitsPerStep{},mineralUnitsPerStep{},precisionUnitsPerStep{};
};
struct FertilityPool{std::uint64_t organic{},mineral{},precision{};};
struct CropPlot{
 std::uint64_t plotId{},cropContentId{},revision{1};CropStage stage{CropStage::Seeded};std::uint64_t stageProgressTicks{},waterUnits{};FertilityPool fertility{};std::uint32_t capabilities{};bool live{true};
};
struct CropEnvironment{
 std::uint64_t plotId{};double light{},temperature{},oxygen{},pressure{},soilSuitability{1},waterSaturation{};bool sealed{},flooded{},pollinated{};double greenhouseBonus{},pollinatorBonus{};
};
struct MaterialCommit{
 std::uint64_t transactionId{},plotId{},waterUnits{},organicUnits{},mineralUnits{},precisionUnits{};
};
struct CropInspection{
 std::uint64_t plotId{};CropStage stage{CropStage::Seeded};std::uint32_t blockers{};double suitability{};std::uint64_t stageProgressTicks{},stageRequiredTicks{};FertilityPool fertility{};std::uint64_t waterUnits{};
};
struct CropSnapshot{std::vector<CropDefinition>definitions;std::vector<CropPlot>plots;std::vector<std::uint64_t>consumedTransactions;};
class CropRuntime{
public:
 bool publish(CropDefinition);
 bool createPlot(CropPlot);
 bool commitMaterials(const MaterialCommit&);
 [[nodiscard]]CropInspection inspect(std::uint64_t plotId,const CropEnvironment&)const;
 bool step(std::uint64_t plotId,const CropEnvironment&,std::uint64_t ticks);
 [[nodiscard]]const CropPlot*find(std::uint64_t plotId)const;
 [[nodiscard]]CropSnapshot snapshot()const;
 bool restore(const CropSnapshot&);
private:
 std::map<std::uint64_t,CropDefinition>defs_;std::map<std::uint64_t,CropPlot>plots_;std::map<std::uint64_t,bool>transactions_;
};
} // namespace elysium::agriculture

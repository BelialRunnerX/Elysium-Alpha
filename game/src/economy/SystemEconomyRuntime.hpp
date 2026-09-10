#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::economy {
enum class EconomyClass:std::uint8_t{Mining,Industrial,Agricultural,Technology,Frontier,Imperial};
enum class TradeSide:std::uint8_t{BuyFromMarket,SellToMarket};
enum class MarketFailure:std::uint8_t{None,Invalid,UnknownItem,InsufficientStock,ProgressionLocked,DuplicateTransaction};
struct EconomyClassDef{EconomyClass type{};std::uint64_t buyDemandTags{},sellSupplyTags{},serviceTags{};double buyWeight{1},sellWeight{1};bool registeredServices{};};
struct CommodityDef{std::uint64_t contentId{},tagMask{},baseValue{},progressionGate{};};
struct MarketStock{std::uint64_t commodityId{},units{},capacity{},restockPerEpoch{};};
struct MarketState{std::uint64_t stableId{},settlementStableId{},revision{1},marketSeed{},restockEpoch{};EconomyClass economyClass{};std::vector<MarketStock>stock;};
struct PriceContext{std::uint64_t commodityId{};TradeSide side{};double standingModifier{1},registrationModifier{1};};
struct PriceBreakdown{std::uint64_t baseValue{};double classWeight{1},stockPressure{1},standingModifier{1},registrationModifier{1};std::uint64_t finalUnitPrice{};};
struct MarketTransaction{std::uint64_t stableId{},marketStableId{},commodityId{},units{},expectedMarketRevision{},progressionLevel{};TradeSide side{};};
struct MarketTradeResult{bool committed{},duplicate{};MarketFailure failure{};std::uint64_t units{},totalValue{},revision{};PriceBreakdown price;bool imperialFileExposure{};};
class SystemEconomyRuntime{public:bool publish(EconomyClassDef);bool publish(CommodityDef);bool createMarket(MarketState);[[nodiscard]]PriceBreakdown price(std::uint64_t marketId,const PriceContext&)const;MarketTradeResult trade(const MarketTransaction&,const PriceContext&);bool restock(std::uint64_t marketId,std::uint64_t epoch);[[nodiscard]]const MarketState*market(std::uint64_t)const;private:std::map<EconomyClass,EconomyClassDef>classes_;std::map<std::uint64_t,CommodityDef>commodities_;std::map<std::uint64_t,MarketState>markets_;std::map<std::uint64_t,MarketTradeResult>processed_;};
} // namespace elysium::economy

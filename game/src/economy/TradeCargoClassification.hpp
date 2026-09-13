#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::economy {
enum class GoodsRegistration:std::uint8_t{LegacyUnknown,Registered,Unregistered,Restricted,Contraband};
enum class TradeLawResult:std::uint8_t{Lawful,AllowedWithMarkup,InspectionRequired,ConfiscationRequired,Rejected};
struct CargoStackClass{std::uint64_t stackStableId{},contentId{},count{},revision{1};GoodsRegistration registration{GoodsRegistration::LegacyUnknown};std::uint64_t registrationAuthorityId{};};
struct TradeCargoPolicy{bool registeredMarketOnly{};bool allowUnregistered{};bool confiscateContraband{};double unregisteredMarkup{1.25},contrabandMarkup{2.0};};
struct CustomsDecisionSnapshot{bool inspection{},warrant{},clearance{};double baseExposure{};};
struct CargoExposureCapability{double exposureMultiplier{1.0};std::uint64_t logisticsCost{};};
struct TradeCargoTransaction{std::uint64_t stableId{},marketStableId{},stackStableId{},expectedStackRevision{};std::uint64_t units{};};
struct TradeCargoOutcome{bool committed{},duplicate{};TradeLawResult law{TradeLawResult::Rejected};double priceMultiplier{1},exposure{};std::uint64_t tradedUnits{},confiscatedUnits{};bool emitStandingOutcome{};};
class TradeCargoClassification{public:bool publish(CargoStackClass);[[nodiscard]]const CargoStackClass*find(std::uint64_t)const;TradeCargoOutcome evaluate(const TradeCargoTransaction&,const TradeCargoPolicy&,const CustomsDecisionSnapshot&,const CargoExposureCapability&);bool rollback(std::uint64_t transactionId);private:std::map<std::uint64_t,CargoStackClass>stacks_;std::map<std::uint64_t,TradeCargoOutcome>processed_;std::map<std::uint64_t,CargoStackClass>before_;};
} // namespace elysium::economy

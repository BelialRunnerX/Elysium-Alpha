#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::economy {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class TradeStage:std::uint8_t{Forecast,Prepare,Arrive,Negotiate,Settle,Diplomacy,Depart,StrategicOutcome,Complete};enum class EconomyIdentity:std::uint8_t{Mining,Industrial,Agricultural,Technology,Frontier,Imperial,RiftEdge};struct TradeMission{StableId id{},origin{},destination{};TradeStage stage{TradeStage::Forecast};EconomyIdentity economy{};std::vector<ContentId>manifest,demands,offers;double routeRisk{};std::uint64_t settlementReceipt{};};class TradeMissionRuntime{public:bool add(TradeMission,std::string&);bool advance(StableId,TradeStage,std::string&);bool settle(StableId,std::uint64_t receipt,std::string&);std::optional<TradeMission>get(StableId)const;private:std::unordered_map<StableId,TradeMission>rows_;};}
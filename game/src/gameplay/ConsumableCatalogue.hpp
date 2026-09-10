#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
namespace elysium::gameplay {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class Effect:std::uint8_t{Oxygen,Shield,Fuel,Medical,Food,ThermalResist,CryoResist,ToxinResist,RadResist,PressureResist,Repair,SealPatch,Flare,ScannerBeacon};struct ConsumableDef{ContentId id{},recipe{},audioEvent{},icon{};Effect effect{};double amount{},durationSeconds{};std::uint64_t compatibleFamilyMask{};};struct UseRequest{std::uint64_t transactionId{};StableId actor{},target{};ContentId item{};double current{},maximum{};};struct UseResult{bool accepted{};double applied{};double newValue{};std::string blocker;Effect effect{};StableId target{};};using Eligibility=std::function<bool(StableId,Effect,std::uint64_t)>;class ConsumableCatalogue{public:bool add(ConsumableDef);const ConsumableDef*find(ContentId)const;UseResult use(const UseRequest&,const Eligibility&);private:std::map<ContentId,ConsumableDef>defs_;std::set<std::uint64_t>transactions_;};}

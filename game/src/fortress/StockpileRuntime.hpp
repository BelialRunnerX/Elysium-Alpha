#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct StockpilePolicy{StableId id{};ContentId filter{};std::uint16_t priority{};std::vector<StableId>takeFrom,giveTo;bool acceptsHazardous{false};};struct Reservation{StableId resource{},owner{};std::uint64_t expiry{};};class StockpileRuntime{public:bool upsert(StockpilePolicy);bool reserve(Reservation,std::string&);void expire(std::uint64_t);std::optional<StableId>chooseDestination(ContentId,const std::vector<StableId>&candidates)const;private:std::unordered_map<StableId,StockpilePolicy>piles_;std::unordered_map<StableId,Reservation>reservations_;};}
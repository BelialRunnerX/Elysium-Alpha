#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
namespace elysium::world {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class CarrierKind:std::uint8_t{Item,Surface,Body,Fluid,Room};struct Contamination{StableId carrier{};CarrierKind kind{};ContentId type{},source{};double load{};std::uint64_t revision{};};class ContaminationRuntime{public:bool set(Contamination,std::string&);double transfer(StableId from,StableId to,double fraction);double clean(StableId,double amount);std::optional<Contamination>get(StableId)const;private:std::unordered_map<StableId,Contamination>rows_;};}
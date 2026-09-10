#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
namespace elysium::industry {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct WorkshopState{StableId id{};ContentId type{};double condition{1};double contamination{},environmentStress{};std::uint64_t cycles{};};struct MaintenanceRequest{StableId workshop{};double urgency{};ContentId spareClass{};};class WorkshopRuntime{public:bool add(WorkshopState,std::string&);void applyUse(StableId,double wear,double contamination,double environment);std::optional<MaintenanceRequest>maintenanceNeed(StableId,double preventive,double failure)const;bool repair(StableId,double restored,std::string&);private:std::unordered_map<StableId,WorkshopState>rows_;};}
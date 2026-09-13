#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;enum class DesignationType:std::uint8_t{Mining,Channel,Smoothing,Construction,Demolition,Salvage,Gathering,UtilityRoute,Quarantine,TrafficRestriction,Burrow,Firebreak,DecompressionExclusion,Evacuation};enum class DesignationStatus:std::uint8_t{Pending,Compiled,Blocked,Cancelled,Completed};struct SpatialTarget{std::int64_t x0{},y0{},z0{},x1{},y1{},z1{};};struct Designation{StableId id{},owner{};DesignationType type{};SpatialTarget target{};std::uint16_t priority{};DesignationStatus status{DesignationStatus::Pending};std::uint64_t worldRevision{};};struct JobRequest{StableId designation{},owner{};DesignationType type{};SpatialTarget target{};std::uint16_t priority{};};class DesignationRuntime{public:bool add(Designation,std::string&);std::optional<JobRequest>compile(StableId,std::uint64_t currentWorldRevision,std::string&);bool cancel(StableId);private:std::unordered_map<StableId,Designation>rows_;};}
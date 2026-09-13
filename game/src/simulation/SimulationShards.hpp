#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::simulation {using StableId=std::uint64_t;enum class ShardTier:std::uint8_t{DirectOperative,ActiveFortress,ActivePlanetRemote,StarSystem,GalaxyHistory};enum class ClockFamily:std::uint8_t{Frame,Tactical,Local,Economy,Ecology,Strategic,Historical};struct ShardRecord{StableId id{};ShardTier tier{};std::uint64_t revision{};std::uint64_t authoritativeTick{};bool hasUnsafeReservation{false};};struct Wakeup{StableId shard{};std::uint64_t tick{};std::uint32_t type{};};class SimulationShardScheduler{public:void setCadence(ShardTier,ClockFamily,std::uint64_t);bool upsert(ShardRecord);bool requestTier(StableId,ShardTier,std::string&);void enqueueWakeup(Wakeup);std::vector<Wakeup> dueWakeups(std::uint64_t,std::size_t maxCount);std::uint64_t cadence(ShardTier,ClockFamily)const;private:std::unordered_map<StableId,ShardRecord> shards_;std::unordered_map<std::uint32_t,std::uint64_t> cadences_;std::vector<Wakeup>wakeups_;};}
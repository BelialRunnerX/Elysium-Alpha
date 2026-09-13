// Intended function: stable-ID objective graph for missions/contracts/tutorials that consumes world facts without owning simulation truth.
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium{
enum class ObjectiveKind:std::uint8_t{Reach,Collect,Craft,Build,Mine,Kill,Scan,Escort,Survive,Trade,Talk,Activate};
enum class ObjectiveState:std::uint8_t{Locked,Active,Complete,Failed};
struct ObjectiveRecord{std::uint64_t objectiveId{};ObjectiveKind kind{};ObjectiveState state{ObjectiveState::Locked};std::uint64_t targetStableId{},targetAddress{};std::uint32_t required{1},progress{};std::vector<std::uint64_t>prerequisites;std::string label;};
class ObjectiveGraph{public:bool add(ObjectiveRecord record);bool activate(std::uint64_t id);bool addProgress(std::uint64_t id,std::uint32_t amount);bool fail(std::uint64_t id);void refreshUnlocks();std::optional<ObjectiveRecord>find(std::uint64_t id)const;std::vector<ObjectiveRecord>visible()const;private:std::unordered_map<std::uint64_t,ObjectiveRecord>records_;};
}

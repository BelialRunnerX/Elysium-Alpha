#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class ObsessionState:std::uint8_t{Triggered,ClaimWorkshop,Demand,Acquire,Create,Resolved,Failed};struct Obsession{StableId id{},citizen{},workshop{};ObsessionState state{ObsessionState::Triggered};std::vector<ContentId>materials,tools,environmentTags;std::uint64_t timeout{};};class ObsessionRuntime{public:bool begin(Obsession,std::string&);bool claim(StableId,StableId,std::string&);bool advance(StableId,ObsessionState,std::string&);bool release(StableId,std::string&);private:std::unordered_map<StableId,Obsession>rows_;std::unordered_map<StableId,StableId>workshopClaims_;};}
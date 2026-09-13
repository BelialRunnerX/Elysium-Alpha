#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace elysium::world { using ContentId=std::uint64_t;using StableId=std::uint64_t; struct EventTemplate{ContentId id{};std::uint32_t version{1};std::uint32_t warningTicks{},activeTicks{},cooldownTicks{},maxConcurrent{1};ContentId rewardTable{},standingHook{};std::vector<ContentId> consequenceAdapters;}; enum class EventPhase:std::uint8_t{Warning,Active,Completed,Cancelled}; struct EventInstance{StableId id{};ContentId templateId{};std::uint64_t address{};std::uint64_t seed{};std::uint64_t startTick{};EventPhase phase{EventPhase::Warning};bool settled{false};}; class PlanetaryEventRuntime{public:bool registerTemplate(EventTemplate,std::string&);std::optional<EventInstance> begin(ContentId,std::uint64_t,std::uint64_t,std::uint64_t,std::string&);std::vector<EventInstance> advance(std::uint64_t);bool settle(StableId,std::string&);private:std::unordered_map<ContentId,EventTemplate> defs_;std::unordered_map<StableId,EventInstance> live_;std::unordered_map<std::uint64_t,std::uint64_t> cooldownUntil_;}; }
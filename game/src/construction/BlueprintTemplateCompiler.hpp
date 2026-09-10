#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>
namespace elysium::construction {
using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class IntentKind:std::uint8_t{Block,Prefab,Machine,MicroMask};enum class Intersection:std::uint8_t{Clear,Replace,Excavate,Obstructed};
struct BlueprintIntent{IntentKind kind{};ContentId content{};std::int32_t x{},y{},z{};std::uint8_t rotation{};ContentId substitutionGroup{};std::uint64_t amount{1};std::map<ContentId,ContentId>configuration;bool containsLiveInventory{},containsFuel{},containsAmmo{},containsInProgressCycle{};};
struct BlueprintTemplate{ContentId id{};std::uint32_t schemaVersion{1};std::vector<BlueprintIntent>intents;};
struct CompileIssue{std::uint32_t index{};std::string code;Intersection intersection{Intersection::Clear};};
struct CompiledIntent{StableId newStableId{};BlueprintIntent intent{};};
struct StampPlan{bool accepted{};std::vector<CompiledIntent>intents;std::map<ContentId,std::uint64_t>cost;std::vector<CompileIssue>issues;};
using StableIdAllocator=std::function<StableId(std::uint32_t)>;using IntersectionProbe=std::function<Intersection(const BlueprintIntent&)>;
StampPlan compileBlueprint(const BlueprintTemplate&,const StableIdAllocator&,const IntersectionProbe&,bool survivalUpFront);
}

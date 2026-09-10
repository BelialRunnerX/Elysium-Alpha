// Intended function: staged construction jobs with explicit material reservations, work requirements and deterministic completion intents.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium{
struct ConstructionMaterial{std::uint32_t itemId{},quantity{};};
enum class ConstructionStage:std::uint8_t{Blueprint,Foundation,Frame,Services,Seal,Finish,Complete,Blocked};
struct ConstructionProject{std::uint64_t projectId{},siteId{},anchorAddress{};ConstructionStage stage{ConstructionStage::Blueprint};std::vector<ConstructionMaterial>materials;float workRequired{},workDone{};std::uint32_t blockerCode{};};
struct ConstructionWorkIntent{std::uint64_t projectId{},workerId{},address{};ConstructionStage stage{};float work{};};
class ConstructionPlanner{public:bool add(ConstructionProject p);std::vector<ConstructionWorkIntent>plan(const std::vector<std::uint64_t>&availableWorkers,std::size_t maxIntents);bool applyWork(std::uint64_t projectId,float work);std::vector<ConstructionProject>projects()const;private:std::vector<ConstructionProject>projects_;};
}

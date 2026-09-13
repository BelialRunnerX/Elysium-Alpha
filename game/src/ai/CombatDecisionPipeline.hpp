#pragma once
#include <cstdint>
#include <span>
#include <vector>
namespace elysium::ai {
struct CombatSense{std::uint64_t actorId{},targetId{},actionId{};double distance{},threat{},cover{},healthFraction{},resourceReadiness{},pathCost{},roleFit{};bool reachable{true};};
struct CombatDecision{std::uint64_t actorId{},targetId{},actionId{};double score{};};
class CombatDecisionPipeline{public:[[nodiscard]]std::vector<CombatDecision>rank(std::span<const CombatSense>candidates)const;};
} // namespace elysium::ai

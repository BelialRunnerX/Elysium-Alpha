// Intended function: stable dialogue graph with explicit conditions/results for NPCs, factions, quests and Court interactions.
#pragma once
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium{
struct DialogueContext{std::uint64_t speakerId{},listenerId{},factionId{},siteId{};float standing{};std::uint32_t flags{};};
struct DialogueChoice{std::uint64_t choiceId{},nextNodeId{};std::string text;float minStanding{-100};std::uint32_t requiredFlags{},setFlags{};float standingDelta{};};
struct DialogueNode{std::uint64_t nodeId{};std::string text;std::vector<DialogueChoice>choices;bool terminal{};};
class DialogueSystem{public:bool add(DialogueNode node);std::optional<DialogueNode>node(std::uint64_t id)const;std::vector<DialogueChoice>availableChoices(std::uint64_t nodeId,const DialogueContext&ctx)const;bool applyChoice(DialogueContext&ctx,const DialogueChoice&choice)const;private:std::unordered_map<std::uint64_t,DialogueNode>nodes_;};
}

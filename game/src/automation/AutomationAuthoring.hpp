#pragma once
#include "automation/TcaRuntime.hpp"
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace elysium::automation {
struct RulePreset{std::uint64_t presetId{};std::vector<TcaRule>rules;std::uint32_t schemaVersion{1};};
struct RuleInspection{StableId ruleId{};SignalId trigger{};ActionId action{};StableId target{};Comparison comparison{};double threshold{};bool valid{};reason::ReasonStack reasons;};
class AutomationAuthoring{public:bool publishPreset(RulePreset p,reason::ReasonStack*r=nullptr);[[nodiscard]]const RulePreset*find(std::uint64_t id)const;[[nodiscard]]RuleInspection inspect(const TcaRule&r)const;private:std::unordered_map<std::uint64_t,RulePreset>presets_;};
} // namespace elysium::automation

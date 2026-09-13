#pragma once
#include <cstdint>
#include <span>
#include <vector>
namespace elysium::base {
struct PowerPolicyInput{std::uint64_t nodeId{};std::uint64_t networkId{};std::uint16_t normalPriority{100};std::uint16_t emergencyPriority{25};bool emergencyCritical{};bool faulted{};bool manuallyIsolated{};};
struct PowerPolicyDecision{std::uint64_t nodeId{};std::uint16_t effectivePriority{};bool isolated{};bool eligible{};};
class PowerPolicy{public:[[nodiscard]] std::vector<PowerPolicyDecision> evaluate(std::span<const PowerPolicyInput> inputs,bool emergencyMode)const;};
} // namespace elysium::base

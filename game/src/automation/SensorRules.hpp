// Intended function: Evaluate finite sensor predicates into deterministic automation signals without arbitrary scripting side effects.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::automation {
struct SensorRuleState {
    std::uint64_t ruleId{};
    std::uint64_t sensorId{};
    double conditionId{};
    double threshold{};
    std::uint64_t outputSignal{};
    std::uint64_t priority{};
};
class SensorRuleStateRegistry {
public:
    bool publish(SensorRuleState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const SensorRuleState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<SensorRuleState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const SensorRuleState& r) noexcept;
    std::vector<SensorRuleState> records_;
};
} // namespace elysium::automation

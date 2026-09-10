// Intended function: Represent deterministic automation nodes/edges, finite rules, stable ports, priorities, and evaluation revisions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::automation {
struct LogicNodeState {
    std::uint64_t nodeId{};
    std::uint64_t kind{};
    std::uint64_t inputMask{};
    std::uint64_t outputMask{};
    std::uint64_t priority{};
    std::uint64_t revision{};
};
class LogicNodeStateRegistry {
public:
    bool publish(LogicNodeState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const LogicNodeState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<LogicNodeState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const LogicNodeState& r) noexcept;
    std::vector<LogicNodeState> records_;
};
} // namespace elysium::automation

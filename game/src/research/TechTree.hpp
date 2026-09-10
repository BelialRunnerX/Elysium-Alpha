// Intended function: Track research nodes, prerequisites, costs, unlock effects, repeatable levels, and deterministic discovery gating.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::research {

struct TechNodeState {
    std::uint64_t techId{};
    std::uint64_t level{};
    double progress{};
    std::uint64_t requiredPoints{};
    std::uint64_t flags{};
    std::uint64_t revision{};
};

class TechNodeStateStore {
public:
    bool upsert(TechNodeState value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const TechNodeState* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<TechNodeState> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const TechNodeState& value) noexcept;
    std::vector<TechNodeState> records_;
};

} // namespace elysium::research

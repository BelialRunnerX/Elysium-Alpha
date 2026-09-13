// Intended function: Implement player-facing mining progress, tool efficiency, deterministic ore rewards, durability, and committed world-edit requests.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::gameplay {

struct MiningAction {
    std::uint64_t actionId{};
    std::uint64_t targetAddressKey{};
    std::uint64_t toolId{};
    double progress{};
    double requiredWork{};
    std::uint64_t flags{};
};

class MiningActionStore {
public:
    bool upsert(MiningAction value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const MiningAction* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<MiningAction> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const MiningAction& value) noexcept;
    std::vector<MiningAction> records_;
};

} // namespace elysium::gameplay

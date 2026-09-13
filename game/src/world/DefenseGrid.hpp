// Intended function: Coordinate defensive emplacements, sectors, ammunition readiness, power readiness, and tactical firing policy.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct DefenseNode {
    std::uint64_t stableId{};
    std::uint64_t sectorId{};
    std::uint64_t ammo{};
    std::uint64_t powerDemand{};
    std::uint64_t readiness{};
    std::uint64_t policyFlags{};
};

class DefenseNodeStore {
public:
    bool upsert(DefenseNode value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const DefenseNode* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<DefenseNode> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const DefenseNode& value) noexcept;
    std::vector<DefenseNode> records_;
};

} // namespace elysium::world

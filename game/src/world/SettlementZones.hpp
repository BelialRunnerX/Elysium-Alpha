// Intended function: Represent bounded policy zones such as stockpiles, hospitals, barracks, farms, traffic, and emergency districts.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct SettlementZone {
    std::uint64_t zoneId{};
    std::uint64_t zoneType{};
    std::uint64_t priority{};
    double capacity{};
    std::uint64_t policyFlags{};
    std::uint64_t revision{};
};

class SettlementZoneStore {
public:
    bool upsert(SettlementZone value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const SettlementZone* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<SettlementZone> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const SettlementZone& value) noexcept;
    std::vector<SettlementZone> records_;
};

} // namespace elysium::world

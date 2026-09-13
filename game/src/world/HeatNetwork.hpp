// Intended function: Model bounded thermal-transfer links, heat sources, sinks, storage, and overheat protection for bases and industry.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct HeatLink {
    std::uint64_t stableId{};
    std::uint64_t networkId{};
    double heatCapacity{};
    double storedHeat{};
    double conductance{};
    std::uint64_t flags{};
};

class HeatLinkStore {
public:
    bool upsert(HeatLink value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const HeatLink* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<HeatLink> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const HeatLink& value) noexcept;
    std::vector<HeatLink> records_;
};

} // namespace elysium::world

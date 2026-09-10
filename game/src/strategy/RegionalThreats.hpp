// Intended function: Aggregate strategic threats by region, escalation, mobility, known intelligence, and response pressure.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::strategy {

struct RegionalThreat {
    double threatId{};
    std::uint64_t regionId{};
    double threatType{};
    double strength{};
    double mobility{};
    double visibility{};
};

class RegionalThreatStore {
public:
    bool upsert(RegionalThreat value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const RegionalThreat* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<RegionalThreat> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const RegionalThreat& value) noexcept;
    std::vector<RegionalThreat> records_;
};

} // namespace elysium::strategy

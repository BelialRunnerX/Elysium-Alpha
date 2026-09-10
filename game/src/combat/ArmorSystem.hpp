// Intended function: Resolve layered armor/suit mitigation, condition loss, penetration, breach flags, and repair requirements.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::combat {

struct ArmorLayer {
    std::uint64_t itemId{};
    std::uint64_t armorClass{};
    double condition{};
    double resistance{};
    double penetrationGuard{};
    std::uint64_t flags{};
};

class ArmorLayerStore {
public:
    bool upsert(ArmorLayer value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const ArmorLayer* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<ArmorLayer> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const ArmorLayer& value) noexcept;
    std::vector<ArmorLayer> records_;
};

} // namespace elysium::combat

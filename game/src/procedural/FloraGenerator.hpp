// Intended function: Generate procedural flora species from body grammar, climate niche, resource traits, defenses, growth form, and reproductive strategy.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct FloraSeed {
    std::uint64_t speciesId{};
    std::uint64_t seed{};
    std::uint64_t bodyPlan{};
    std::uint64_t climateNiche{};
    std::uint64_t resourceTrait{};
    double rarity{};
};
class FloraSeedTable {
public:
 bool set(FloraSeed value); bool remove(std::uint64_t id);
 [[nodiscard]] const FloraSeed* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<FloraSeed> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const FloraSeed& value) noexcept; std::vector<FloraSeed> rows_;
};
}

// Intended function: Generate procedural fauna from body grammar, locomotion, diet, senses, temperament, attacks, defenses, and habitat niche.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct FaunaSeed {
    std::uint64_t speciesId{};
    std::uint64_t seed{};
    std::uint64_t bodyPlan{};
    std::uint64_t locomotion{};
    std::uint64_t diet{};
    std::uint64_t threatTier{};
};
class FaunaSeedTable {
public:
 bool set(FaunaSeed value); bool remove(std::uint64_t id);
 [[nodiscard]] const FaunaSeed* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<FaunaSeed> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const FaunaSeed& value) noexcept; std::vector<FaunaSeed> rows_;
};
}

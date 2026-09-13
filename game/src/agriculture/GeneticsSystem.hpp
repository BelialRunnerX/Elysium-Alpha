// Intended function: Track deterministic crop/livestock genetic lines, traits, mutation, selection, breeding, and lineage identity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::agriculture {
struct GeneticLine {
    std::uint64_t lineId{};
    std::uint64_t speciesId{};
    std::uint64_t traitHash{};
    std::uint64_t generation{};
    double fitness{};
    double stability{};
};
class GeneticLineStore {
public:
 bool put(GeneticLine v); bool erase(std::uint64_t id);
 [[nodiscard]] const GeneticLine* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<GeneticLine>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const GeneticLine& v) noexcept; std::vector<GeneticLine> values_;
};
} // namespace elysium::agriculture

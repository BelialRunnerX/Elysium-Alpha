#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Compute dietary diversity, nutrient coverage, contamination, freshness, and long-term health effects.
struct NutritionQualityInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct NutritionQualitySnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class NutritionQualityModel {
public:
 bool update(const NutritionQualityInput& input);
 const NutritionQualitySnapshot* get(std::uint64_t keyId) const;
 std::vector<NutritionQualitySnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,NutritionQualitySnapshot> data_;
};

}

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance freshness, refrigeration, contamination, preservation, and spoilage conversion for stored food.
struct FoodSpoilageInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct FoodSpoilageSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class FoodSpoilageModel {
public:
 bool update(const FoodSpoilageInput& input);
 const FoodSpoilageSnapshot* get(std::uint64_t keyId) const;
 std::vector<FoodSpoilageSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,FoodSpoilageSnapshot> data_;
};

}

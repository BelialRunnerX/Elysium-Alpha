#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent cultural cuisine recipes, ingredient substitutions, nutrition, morale value, preparation, and skill requirements.
struct RecipeCuisineInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct RecipeCuisineSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class RecipeCuisineModel {
public:
 bool update(const RecipeCuisineInput& input);
 const RecipeCuisineSnapshot* get(std::uint64_t keyId) const;
 std::vector<RecipeCuisineSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,RecipeCuisineSnapshot> data_;
};

}

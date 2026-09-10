#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Schedule meals from population diets, available ingredients, nutrition targets, culture, and kitchen capacity.
struct KitchenPlannerInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct KitchenPlannerSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class KitchenPlannerModel {
public:
 bool update(const KitchenPlannerInput& input);
 const KitchenPlannerSnapshot* get(std::uint64_t keyId) const;
 std::vector<KitchenPlannerSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,KitchenPlannerSnapshot> data_;
};

}

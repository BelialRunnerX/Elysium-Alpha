#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Coordinate stable pack/herd membership, leadership, formations, hunting, fleeing, and regrouping intents.
struct PackBehaviorInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct PackBehaviorSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class PackBehaviorModel {
public:
 bool update(const PackBehaviorInput& input);
 const PackBehaviorSnapshot* get(std::uint64_t keyId) const;
 std::vector<PackBehaviorSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,PackBehaviorSnapshot> data_;
};

}

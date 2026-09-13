#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Model technology sharing, theft, trade, teaching, reverse engineering, and civilization adoption at strategic LOD.
struct TechDiffusionInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct TechDiffusionSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class TechDiffusionModel {
public:
 bool update(const TechDiffusionInput& input);
 const TechDiffusionSnapshot* get(std::uint64_t keyId) const;
 std::vector<TechDiffusionSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,TechDiffusionSnapshot> data_;
};

}

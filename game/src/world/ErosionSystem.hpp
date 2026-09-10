#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Apply slow bounded terrain-change requests from wind, water, ice, collapse, and player terraforming.
struct ErosionSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ErosionSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ErosionSystemModel {
public:
 bool update(const ErosionSystemInput& input);
 const ErosionSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<ErosionSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ErosionSystemSnapshot> data_;
};

}

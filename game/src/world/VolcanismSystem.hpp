#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track magma pressure, vents, eruptions, ash, lava, heat, gas, and long-term terrain/biome effects.
struct VolcanismSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct VolcanismSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class VolcanismSystemModel {
public:
 bool update(const VolcanismSystemInput& input);
 const VolcanismSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<VolcanismSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,VolcanismSystemSnapshot> data_;
};

}

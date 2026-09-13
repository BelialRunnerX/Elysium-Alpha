#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Compute radial pressure, fragmentation, heat, cover attenuation, structural load, and presentation events.
struct ExplosionSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ExplosionSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ExplosionSystemModel {
public:
 bool update(const ExplosionSystemInput& input);
 const ExplosionSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<ExplosionSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ExplosionSystemSnapshot> data_;
};

}

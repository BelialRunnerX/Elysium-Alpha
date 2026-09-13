#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Resolve vehicle sweep, suspension contact, obstacle, slope, and impact queries without owning vehicle simulation.
struct VehicleCollisionInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct VehicleCollisionSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class VehicleCollisionModel {
public:
 bool update(const VehicleCollisionInput& input);
 const VehicleCollisionSnapshot* get(std::uint64_t keyId) const;
 std::vector<VehicleCollisionSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,VehicleCollisionSnapshot> data_;
};

}

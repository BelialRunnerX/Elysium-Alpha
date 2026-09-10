#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Register stable ship modules, slots, stats, power/heat/fuel, crew, damage behavior, recipes, and visual hooks.
struct ShipModuleCatalogueInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ShipModuleCatalogueSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ShipModuleCatalogueModel {
public:
 bool update(const ShipModuleCatalogueInput& input);
 const ShipModuleCatalogueSnapshot* get(std::uint64_t keyId) const;
 std::vector<ShipModuleCatalogueSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ShipModuleCatalogueSnapshot> data_;
};

}

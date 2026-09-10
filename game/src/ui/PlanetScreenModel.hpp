#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build planet survey, biomes, settlements, weather, resources, hazards, claims, missions, and orbital assets projections.
struct PlanetScreenModelInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct PlanetScreenModelSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class PlanetScreenModelModel {
public:
 bool update(const PlanetScreenModelInput& input);
 const PlanetScreenModelSnapshot* get(std::uint64_t keyId) const;
 std::vector<PlanetScreenModelSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,PlanetScreenModelSnapshot> data_;
};

}

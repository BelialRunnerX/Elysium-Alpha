#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track bounded soil moisture, fertility, salinity, contamination, structure, and amendment needs for cultivated cells.
struct SoilSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct SoilSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class SoilSystemModel {
public:
 bool update(const SoilSystemInput& input);
 const SoilSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<SoilSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,SoilSystemSnapshot> data_;
};

}

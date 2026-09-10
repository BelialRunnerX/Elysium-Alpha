#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track sparse stable settlement/site anchors per planet without allocating dense planet-global grids.
struct PlanetSettlementIndexInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct PlanetSettlementIndexSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class PlanetSettlementIndexModel {
public:
 bool update(const PlanetSettlementIndexInput& input);
 const PlanetSettlementIndexSnapshot* get(std::uint64_t keyId) const;
 std::vector<PlanetSettlementIndexSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,PlanetSettlementIndexSnapshot> data_;
};

}

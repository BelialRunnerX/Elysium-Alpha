#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Query local gravity direction/magnitude from planet, ship, station, anomaly, and artificial-gravity summaries.
struct GravityFieldInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct GravityFieldSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class GravityFieldModel {
public:
 bool update(const GravityFieldInput& input);
 const GravityFieldSnapshot* get(std::uint64_t keyId) const;
 std::vector<GravityFieldSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,GravityFieldSnapshot> data_;
};

}

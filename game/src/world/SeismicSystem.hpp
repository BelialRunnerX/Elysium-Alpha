#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate deterministic quake events and structural-load requests from tectonics, mining, explosions, and anomalies.
struct SeismicSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct SeismicSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class SeismicSystemModel {
public:
 bool update(const SeismicSystemInput& input);
 const SeismicSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<SeismicSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,SeismicSystemSnapshot> data_;
};

}

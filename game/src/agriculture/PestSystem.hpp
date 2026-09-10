#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Model pest populations, crop damage, predators, treatments, quarantine, and seasonal pressure.
struct PestSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct PestSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class PestSystemModel {
public:
 bool update(const PestSystemInput& input);
 const PestSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<PestSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,PestSystemSnapshot> data_;
};

}

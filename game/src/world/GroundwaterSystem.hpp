#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track bounded aquifer pressure, contamination, extraction, recharge, and seepage around active regions.
struct GroundwaterSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct GroundwaterSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class GroundwaterSystemModel {
public:
 bool update(const GroundwaterSystemInput& input);
 const GroundwaterSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<GroundwaterSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,GroundwaterSystemSnapshot> data_;
};

}

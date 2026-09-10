#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate research progress and unlock candidates from captured machines, ships, artifacts, and enemy equipment.
struct ReverseEngineeringInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ReverseEngineeringSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ReverseEngineeringModel {
public:
 bool update(const ReverseEngineeringInput& input);
 const ReverseEngineeringSnapshot* get(std::uint64_t keyId) const;
 std::vector<ReverseEngineeringSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ReverseEngineeringSnapshot> data_;
};

}

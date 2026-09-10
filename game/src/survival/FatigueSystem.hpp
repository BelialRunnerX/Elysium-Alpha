#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track exertion and sleep debt and expose movement, combat, work, and cognition penalties without owning schedules.
struct FatigueSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct FatigueSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class FatigueSystemModel {
public:
 bool update(const FatigueSystemInput& input);
 const FatigueSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<FatigueSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,FatigueSystemSnapshot> data_;
};

}

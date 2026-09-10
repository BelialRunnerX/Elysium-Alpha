#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track leaders, offices, succession rules, legitimacy, coups, elections, appointments, and command authority.
struct LeadershipSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct LeadershipSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class LeadershipSystemModel {
public:
 bool update(const LeadershipSystemInput& input);
 const LeadershipSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<LeadershipSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,LeadershipSystemSnapshot> data_;
};

}

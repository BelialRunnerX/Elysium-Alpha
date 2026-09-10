#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent faction values, taboos, priorities, legitimacy, ideological drift, and compatibility with other groups.
struct IdeologySystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct IdeologySystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class IdeologySystemModel {
public:
 bool update(const IdeologySystemInput& input);
 const IdeologySystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<IdeologySystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,IdeologySystemSnapshot> data_;
};

}

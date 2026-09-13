#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent faction and settlement laws, jurisdiction, prohibited acts, penalties, exceptions, and enforcement policies.
struct LawSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct LawSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class LawSystemModel {
public:
 bool update(const LawSystemInput& input);
 const LawSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<LawSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,LawSystemSnapshot> data_;
};

}

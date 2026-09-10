#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Aggregate personal needs, relationships, safety, environment, leadership, victories, and trauma into bounded morale state.
struct MoraleSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct MoraleSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class MoraleSystemModel {
public:
 bool update(const MoraleSystemInput& input);
 const MoraleSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<MoraleSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,MoraleSystemSnapshot> data_;
};

}

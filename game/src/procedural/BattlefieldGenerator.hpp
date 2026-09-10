#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate persistent battle aftermath sites from participants, weapons, terrain, casualties, wreckage, and Chronicle facts.
struct BattlefieldGeneratorInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct BattlefieldGeneratorSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class BattlefieldGeneratorModel {
public:
 bool update(const BattlefieldGeneratorInput& input);
 const BattlefieldGeneratorSnapshot* get(std::uint64_t keyId) const;
 std::vector<BattlefieldGeneratorSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,BattlefieldGeneratorSnapshot> data_;
};

}

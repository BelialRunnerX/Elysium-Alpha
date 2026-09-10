#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Resolve capsule/shape movement queries against voxel and object collision surfaces for direct-operative control.
struct CharacterCollisionInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct CharacterCollisionSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class CharacterCollisionModel {
public:
 bool update(const CharacterCollisionInput& input);
 const CharacterCollisionSnapshot* get(std::uint64_t keyId) const;
 std::vector<CharacterCollisionSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,CharacterCollisionSnapshot> data_;
};

}

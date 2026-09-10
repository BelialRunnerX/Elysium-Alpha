#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Translate explosions, impacts, structural failure, and mining events into bounded microvoxel damage requests.
struct DestructionImpulseInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct DestructionImpulseSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class DestructionImpulseModel {
public:
 bool update(const DestructionImpulseInput& input);
 const DestructionImpulseSnapshot* get(std::uint64_t keyId) const;
 std::vector<DestructionImpulseSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,DestructionImpulseSnapshot> data_;
};

}

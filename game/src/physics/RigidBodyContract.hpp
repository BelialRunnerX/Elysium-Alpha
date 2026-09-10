#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Define backend-neutral rigid-body state and deterministic force/impulse command seams for movable gameplay objects.
struct RigidBodyContractInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct RigidBodyContractSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class RigidBodyContractModel {
public:
 bool update(const RigidBodyContractInput& input);
 const RigidBodyContractSnapshot* get(std::uint64_t keyId) const;
 std::vector<RigidBodyContractSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,RigidBodyContractSnapshot> data_;
};

}

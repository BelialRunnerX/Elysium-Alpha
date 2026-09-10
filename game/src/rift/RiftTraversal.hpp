#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent transit manifests and outcomes through unstable Rift links while preserving stable identities.
struct RiftTraversalInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct RiftTraversalSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class RiftTraversalModel {
public:
 bool update(const RiftTraversalInput& input);
 const RiftTraversalSnapshot* get(std::uint64_t keyId) const;
 std::vector<RiftTraversalSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,RiftTraversalSnapshot> data_;
};

}

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track Rift opening stability, energy demand, destination coherence, hazards, collapse, and sealing operations.
struct RiftStabilityInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct RiftStabilitySnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class RiftStabilityModel {
public:
 bool update(const RiftStabilityInput& input);
 const RiftStabilitySnapshot* get(std::uint64_t keyId) const;
 std::vector<RiftStabilitySnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,RiftStabilitySnapshot> data_;
};

}

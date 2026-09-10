#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Model Rift-origin flora/fauna/anomaly pressure and contamination around active breaches.
struct RiftEcologyInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct RiftEcologySnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class RiftEcologyModel {
public:
 bool update(const RiftEcologyInput& input);
 const RiftEcologySnapshot* get(std::uint64_t keyId) const;
 std::vector<RiftEcologySnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,RiftEcologySnapshot> data_;
};

}

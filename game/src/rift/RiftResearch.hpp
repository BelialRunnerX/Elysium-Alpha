#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track experiments, samples, observations, unlocks, accidents, and Chronicle-worthy Rift discoveries.
struct RiftResearchInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct RiftResearchSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class RiftResearchModel {
public:
 bool update(const RiftResearchInput& input);
 const RiftResearchSnapshot* get(std::uint64_t keyId) const;
 std::vector<RiftResearchSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,RiftResearchSnapshot> data_;
};

}

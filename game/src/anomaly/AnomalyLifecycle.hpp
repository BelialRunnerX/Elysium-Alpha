#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance anomaly activation, stability, hazards, rewards, propagation, and collapse from deterministic state.
struct AnomalyLifecycleInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct AnomalyLifecycleSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class AnomalyLifecycleModel {
public:
 bool update(const AnomalyLifecycleInput& input);
 const AnomalyLifecycleSnapshot* get(std::uint64_t keyId) const;
 std::vector<AnomalyLifecycleSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,AnomalyLifecycleSnapshot> data_;
};

}

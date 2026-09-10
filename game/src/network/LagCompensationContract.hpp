#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Define bounded historical query snapshots for future authoritative hit validation without storing transient ECS IDs.
struct LagCompensationContractInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct LagCompensationContractSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class LagCompensationContractModel {
public:
 bool update(const LagCompensationContractInput& input);
 const LagCompensationContractSnapshot* get(std::uint64_t keyId) const;
 std::vector<LagCompensationContractSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,LagCompensationContractSnapshot> data_;
};

}

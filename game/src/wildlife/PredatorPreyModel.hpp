#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance bounded population pressure between trophic groups using remote ecology summaries.
struct PredatorPreyModelInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct PredatorPreyModelSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class PredatorPreyModelModel {
public:
 bool update(const PredatorPreyModelInput& input);
 const PredatorPreyModelSnapshot* get(std::uint64_t keyId) const;
 std::vector<PredatorPreyModelSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,PredatorPreyModelSnapshot> data_;
};

}

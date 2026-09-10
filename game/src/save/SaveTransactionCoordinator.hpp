#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Coordinate atomic publication of shard, chunk, stable-object, strategic, and sidecar records through one generation.
struct SaveTransactionCoordinatorInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct SaveTransactionCoordinatorSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class SaveTransactionCoordinatorModel {
public:
 bool update(const SaveTransactionCoordinatorInput& input);
 const SaveTransactionCoordinatorSnapshot* get(std::uint64_t keyId) const;
 std::vector<SaveTransactionCoordinatorSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,SaveTransactionCoordinatorSnapshot> data_;
};

}

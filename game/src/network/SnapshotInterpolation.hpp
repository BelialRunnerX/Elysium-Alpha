#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Define presentation-only interpolation buffers for future replicated transforms without changing authoritative state.
struct SnapshotInterpolationInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct SnapshotInterpolationSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class SnapshotInterpolationModel {
public:
 bool update(const SnapshotInterpolationInput& input);
 const SnapshotInterpolationSnapshot* get(std::uint64_t keyId) const;
 std::vector<SnapshotInterpolationSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,SnapshotInterpolationSnapshot> data_;
};

}

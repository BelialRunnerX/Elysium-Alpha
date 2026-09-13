#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Encode/decode stable entity component snapshots while excluding transient registry identity and renderer handles.
struct EntitySnapshotCodecInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct EntitySnapshotCodecSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class EntitySnapshotCodecModel {
public:
 bool update(const EntitySnapshotCodecInput& input);
 const EntitySnapshotCodecSnapshot* get(std::uint64_t keyId) const;
 std::vector<EntitySnapshotCodecSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,EntitySnapshotCodecSnapshot> data_;
};

}

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Partition persistent mod-owned records by stable namespace and version while retaining removal/migration tombstones.
struct ModSaveNamespaceInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ModSaveNamespaceSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ModSaveNamespaceModel {
public:
 bool update(const ModSaveNamespaceInput& input);
 const ModSaveNamespaceSnapshot* get(std::uint64_t keyId) const;
 std::vector<ModSaveNamespaceSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ModSaveNamespaceSnapshot> data_;
};

}

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Apply deterministic versioned mod-state migrations and reject missing or cyclic migration paths.
struct ModMigrationSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ModMigrationSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ModMigrationSystemModel {
public:
 bool update(const ModMigrationSystemInput& input);
 const ModMigrationSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<ModMigrationSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ModMigrationSystemSnapshot> data_;
};

}

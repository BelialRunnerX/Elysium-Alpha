#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent seasonal wildlife migration corridors from climate, food, hazards, and terrain summaries.
struct MigrationRoutesInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct MigrationRoutesSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class MigrationRoutesModel {
public:
 bool update(const MigrationRoutesInput& input);
 const MigrationRoutesSnapshot* get(std::uint64_t keyId) const;
 std::vector<MigrationRoutesSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,MigrationRoutesSnapshot> data_;
};

}

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Register data-driven ability costs, cooldowns, targeting, effects, tags, animation/audio hooks, and AI utility metadata.
struct AbilityCatalogueInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct AbilityCatalogueSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class AbilityCatalogueModel {
public:
 bool update(const AbilityCatalogueInput& input);
 const AbilityCatalogueSnapshot* get(std::uint64_t keyId) const;
 std::vector<AbilityCatalogueSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,AbilityCatalogueSnapshot> data_;
};

}

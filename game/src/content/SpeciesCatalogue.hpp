#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Register stable sapient and fauna species definitions, traits, visuals, habitats, loot, domestication, and lore references.
struct SpeciesCatalogueInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct SpeciesCatalogueSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class SpeciesCatalogueModel {
public:
 bool update(const SpeciesCatalogueInput& input);
 const SpeciesCatalogueSnapshot* get(std::uint64_t keyId) const;
 std::vector<SpeciesCatalogueSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,SpeciesCatalogueSnapshot> data_;
};

}

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Register stable culture definitions for values, aesthetics, cuisine, institutions, names, rituals, and social rules.
struct CultureCatalogueInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct CultureCatalogueSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class CultureCatalogueModel {
public:
 bool update(const CultureCatalogueInput& input);
 const CultureCatalogueSnapshot* get(std::uint64_t keyId) const;
 std::vector<CultureCatalogueSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,CultureCatalogueSnapshot> data_;
};

}

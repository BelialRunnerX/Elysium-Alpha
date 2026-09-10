#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Register data-driven status effects, stacking rules, duration, modifiers, cures, immunity, and presentation hooks.
struct StatusEffectCatalogueInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct StatusEffectCatalogueSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class StatusEffectCatalogueModel {
public:
 bool update(const StatusEffectCatalogueInput& input);
 const StatusEffectCatalogueSnapshot* get(std::uint64_t keyId) const;
 std::vector<StatusEffectCatalogueSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,StatusEffectCatalogueSnapshot> data_;
};

}

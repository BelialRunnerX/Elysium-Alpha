#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track sparse territory anchors, overlap, migration pressure, nesting, food access, and threat response.
struct TerritorySystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct TerritorySystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class TerritorySystemModel {
public:
 bool update(const TerritorySystemInput& input);
 const TerritorySystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<TerritorySystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,TerritorySystemSnapshot> data_;
};

}

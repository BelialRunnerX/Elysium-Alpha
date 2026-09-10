#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Aggregate long-lived cultural memories from wars, disasters, heroes, betrayals, discoveries, and migrations.
struct CulturalMemoryInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct CulturalMemorySnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class CulturalMemoryModel {
public:
 bool update(const CulturalMemoryInput& input);
 const CulturalMemorySnapshot* get(std::uint64_t keyId) const;
 std::vector<CulturalMemorySnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,CulturalMemorySnapshot> data_;
};

}

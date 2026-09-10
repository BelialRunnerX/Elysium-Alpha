#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate damaged ship/station layouts, failure history, hazards, survivors, loot, logs, and salvage opportunities.
struct DerelictGeneratorInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct DerelictGeneratorSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class DerelictGeneratorModel {
public:
 bool update(const DerelictGeneratorInput& input);
 const DerelictGeneratorSnapshot* get(std::uint64_t keyId) const;
 std::vector<DerelictGeneratorSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,DerelictGeneratorSnapshot> data_;
};

}

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate exploration/combat/social encounters from biome, faction, hazard, time, progression, and nearby history.
struct EncounterGeneratorInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct EncounterGeneratorSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class EncounterGeneratorModel {
public:
 bool update(const EncounterGeneratorInput& input);
 const EncounterGeneratorSnapshot* get(std::uint64_t keyId) const;
 std::vector<EncounterGeneratorSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,EncounterGeneratorSnapshot> data_;
};

}

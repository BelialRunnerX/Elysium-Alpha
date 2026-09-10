#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate context-sensitive narrative beats from world facts, relationships, conflicts, discoveries, and faction agendas.
struct NarrativeBeatGeneratorInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct NarrativeBeatGeneratorSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class NarrativeBeatGeneratorModel {
public:
 bool update(const NarrativeBeatGeneratorInput& input);
 const NarrativeBeatGeneratorSnapshot* get(std::uint64_t keyId) const;
 std::vector<NarrativeBeatGeneratorSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,NarrativeBeatGeneratorSnapshot> data_;
};

}

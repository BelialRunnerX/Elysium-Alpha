#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate bounded rumors with source, truth confidence, distortion, propagation, expiry, and discovery hooks.
struct RumorGeneratorInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct RumorGeneratorSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class RumorGeneratorModel {
public:
 bool update(const RumorGeneratorInput& input);
 const RumorGeneratorSnapshot* get(std::uint64_t keyId) const;
 std::vector<RumorGeneratorSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,RumorGeneratorSnapshot> data_;
};

}

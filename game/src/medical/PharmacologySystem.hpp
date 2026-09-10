#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track medicine doses, duration, interactions, toxicity, resistance, and treatment intents.
struct PharmacologySystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct PharmacologySystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class PharmacologySystemModel {
public:
 bool update(const PharmacologySystemInput& input);
 const PharmacologySystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<PharmacologySystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,PharmacologySystemSnapshot> data_;
};

}

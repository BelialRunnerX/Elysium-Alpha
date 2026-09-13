#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track stable crop varieties, seed reserves, viability, provenance, breeding lines, and distribution requests.
struct SeedBankInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct SeedBankSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class SeedBankModel {
public:
 bool update(const SeedBankInput& input);
 const SeedBankSnapshot* get(std::uint64_t keyId) const;
 std::vector<SeedBankSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,SeedBankSnapshot> data_;
};

}

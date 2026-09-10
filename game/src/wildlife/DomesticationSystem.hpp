#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track taming, trust, training, breeding, work roles, ownership, and escape risk for domesticable fauna.
struct DomesticationSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct DomesticationSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class DomesticationSystemModel {
public:
 bool update(const DomesticationSystemInput& input);
 const DomesticationSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<DomesticationSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,DomesticationSystemSnapshot> data_;
};

}

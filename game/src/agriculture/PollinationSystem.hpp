#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Aggregate pollinator activity, wind, greenhouse support, flowering windows, and crop fertility.
struct PollinationSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct PollinationSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class PollinationSystemModel {
public:
 bool update(const PollinationSystemInput& input);
 const PollinationSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<PollinationSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,PollinationSystemSnapshot> data_;
};

}

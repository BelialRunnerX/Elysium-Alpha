#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track replacement limbs/organs, implants, compatibility, maintenance, performance, and damage state.
struct ProstheticsSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ProstheticsSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ProstheticsSystemModel {
public:
 bool update(const ProstheticsSystemInput& input);
 const ProstheticsSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<ProstheticsSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ProstheticsSystemSnapshot> data_;
};

}

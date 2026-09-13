#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent inheritable wildlife traits and bounded mutation/breeding outcomes without individual genome simulation at strategic LOD.
struct GeneticsSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct GeneticsSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class GeneticsSystemModel {
public:
 bool update(const GeneticsSystemInput& input);
 const GeneticsSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<GeneticsSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,GeneticsSystemSnapshot> data_;
};

}

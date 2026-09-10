#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate and update stable river channels, flow summaries, flooding, erosion hooks, and settlement water access.
struct RiverSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct RiverSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class RiverSystemModel {
public:
 bool update(const RiverSystemInput& input);
 const RiverSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<RiverSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,RiverSystemSnapshot> data_;
};

}

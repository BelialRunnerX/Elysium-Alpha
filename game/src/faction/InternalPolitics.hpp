#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track interest groups, support, grievances, reforms, corruption, crises, and policy pressure within factions.
struct InternalPoliticsInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct InternalPoliticsSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class InternalPoliticsModel {
public:
 bool update(const InternalPoliticsInput& input);
 const InternalPoliticsSnapshot* get(std::uint64_t keyId) const;
 std::vector<InternalPoliticsSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,InternalPoliticsSnapshot> data_;
};

}

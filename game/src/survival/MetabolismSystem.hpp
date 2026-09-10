#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Model calorie, hydration, oxygen, temperature, exertion, sleep, and stress demands from authoritative actor summaries.
struct MetabolismSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct MetabolismSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class MetabolismSystemModel {
public:
 bool update(const MetabolismSystemInput& input);
 const MetabolismSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<MetabolismSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,MetabolismSystemSnapshot> data_;
};

}

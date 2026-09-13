#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track suit oxygen, scrubber, thermal, radiation, power, breach, seal, and reserve state for direct-operative play.
struct SuitLifeSupportInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct SuitLifeSupportSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class SuitLifeSupportModel {
public:
 bool update(const SuitLifeSupportInput& input);
 const SuitLifeSupportSnapshot* get(std::uint64_t keyId) const;
 std::vector<SuitLifeSupportSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,SuitLifeSupportSnapshot> data_;
};

}

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Encode strategic populations, economies, diplomacy, fleets, contracts, history, and remote sites with versioning.
struct StrategicStateCodecInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct StrategicStateCodecSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class StrategicStateCodecModel {
public:
 bool update(const StrategicStateCodecInput& input);
 const StrategicStateCodecSnapshot* get(std::uint64_t keyId) const;
 std::vector<StrategicStateCodecSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,StrategicStateCodecSnapshot> data_;
};

}

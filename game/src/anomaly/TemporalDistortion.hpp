#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent local time-scale modifiers as gameplay/simulation requests without changing global deterministic tick order.
struct TemporalDistortionInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct TemporalDistortionSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class TemporalDistortionModel {
public:
 bool update(const TemporalDistortionInput& input);
 const TemporalDistortionSnapshot* get(std::uint64_t keyId) const;
 std::vector<TemporalDistortionSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,TemporalDistortionSnapshot> data_;
};

}

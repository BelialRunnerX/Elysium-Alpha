#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Accumulate radiation, toxins, pressure, vacuum, heat, cold, and contamination exposure with equipment mitigation hooks.
struct ExposureSystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ExposureSystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ExposureSystemModel {
public:
 bool update(const ExposureSystemInput& input);
 const ExposureSystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<ExposureSystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ExposureSystemSnapshot> data_;
};

}

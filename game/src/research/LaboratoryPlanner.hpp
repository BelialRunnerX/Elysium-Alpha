#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Allocate researchers, labs, equipment, power, samples, and priorities across active projects.
struct LaboratoryPlannerInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct LaboratoryPlannerSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class LaboratoryPlannerModel {
public:
 bool update(const LaboratoryPlannerInput& input);
 const LaboratoryPlannerSnapshot* get(std::uint64_t keyId) const;
 std::vector<LaboratoryPlannerSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,LaboratoryPlannerSnapshot> data_;
};

}

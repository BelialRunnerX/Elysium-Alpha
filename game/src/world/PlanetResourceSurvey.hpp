#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Aggregate discovered ore, water, organics, energy, archaeology, and hazard indicators into stable survey summaries.
struct PlanetResourceSurveyInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct PlanetResourceSurveySnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class PlanetResourceSurveyModel {
public:
 bool update(const PlanetResourceSurveyInput& input);
 const PlanetResourceSurveySnapshot* get(std::uint64_t keyId) const;
 std::vector<PlanetResourceSurveySnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,PlanetResourceSurveySnapshot> data_;
};

}

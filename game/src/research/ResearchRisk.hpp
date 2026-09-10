#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Estimate experiment hazard, contamination, equipment damage, injury, and anomaly escalation before execution.
struct ResearchRiskInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ResearchRiskSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ResearchRiskModel {
public:
 bool update(const ResearchRiskInput& input);
 const ResearchRiskSnapshot* get(std::uint64_t keyId) const;
 std::vector<ResearchRiskSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ResearchRiskSnapshot> data_;
};

}

#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent bombardment authorization, targeting limits, collateral-risk constraints, and political consequences.
struct OrbitalBombardmentPolicyOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct OrbitalBombardmentPolicyData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class OrbitalBombardmentPolicyStore { public: bool apply(const OrbitalBombardmentPolicyOp&); bool erase(std::uint64_t); const OrbitalBombardmentPolicyData* find(std::uint64_t) const; std::vector<OrbitalBombardmentPolicyData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,OrbitalBombardmentPolicyData> data_; };
}

#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track artifact scanning, hypotheses, handling, transport, experimentation, risks, unlocks, and historical interpretation.
struct ArtifactInvestigationOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ArtifactInvestigationData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ArtifactInvestigationStore { public: bool apply(const ArtifactInvestigationOp&); bool erase(std::uint64_t); const ArtifactInvestigationData* find(std::uint64_t) const; std::vector<ArtifactInvestigationData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ArtifactInvestigationData> data_; };
}

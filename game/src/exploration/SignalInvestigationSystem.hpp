#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track unidentified-signal triangulation, source hypotheses, route planning, discoveries, and false-positive resolution.
struct SignalInvestigationSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SignalInvestigationSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SignalInvestigationSystemStore { public: bool apply(const SignalInvestigationSystemOp&); bool erase(std::uint64_t); const SignalInvestigationSystemData* find(std::uint64_t) const; std::vector<SignalInvestigationSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SignalInvestigationSystemData> data_; };
}

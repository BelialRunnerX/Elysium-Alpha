#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate disputes, insults, accidents, border incidents, espionage discoveries, trade conflicts, and de-escalation options.
struct DiplomaticIncidentGeneratorCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct DiplomaticIncidentGeneratorState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class DiplomaticIncidentGeneratorSystem { public: bool submit(const DiplomaticIncidentGeneratorCommand&); const DiplomaticIncidentGeneratorState* find(std::uint64_t) const; std::vector<DiplomaticIncidentGeneratorState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DiplomaticIncidentGeneratorState> map_; };
}

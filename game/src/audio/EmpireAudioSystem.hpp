#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate Imperial announcement, Register Action, Praetor, facility, combat, and campaign musical/audio motifs.
struct EmpireAudioSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct EmpireAudioSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class EmpireAudioSystemSystem { public: bool submit(const EmpireAudioSystemCommand&); const EmpireAudioSystemState* find(std::uint64_t) const; std::vector<EmpireAudioSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,EmpireAudioSystemState> map_; };
}

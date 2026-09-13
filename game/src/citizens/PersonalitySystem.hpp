#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent stable citizen personality dimensions that influence work, social, risk, combat, and negotiation choices.
struct PersonalitySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PersonalitySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PersonalitySystemStore { public: bool apply(const PersonalitySystemOp&); bool erase(std::uint64_t); const PersonalitySystemData* find(std::uint64_t) const; std::vector<PersonalitySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PersonalitySystemData> data_; };
}

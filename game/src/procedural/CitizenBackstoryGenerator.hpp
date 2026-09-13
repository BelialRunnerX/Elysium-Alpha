#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate bounded citizen origins, family, profession, migrations, traumas, achievements, affiliations, and motivations.
struct CitizenBackstoryGeneratorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CitizenBackstoryGeneratorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CitizenBackstoryGeneratorStore { public: bool apply(const CitizenBackstoryGeneratorOp&); bool erase(std::uint64_t); const CitizenBackstoryGeneratorData* find(std::uint64_t) const; std::vector<CitizenBackstoryGeneratorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CitizenBackstoryGeneratorData> data_; };
}

#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track settlement sanitation capacity, waste pickup, contamination, hygiene access, and disease-risk modifiers.
struct SanitationServiceOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SanitationServiceData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SanitationServiceStore { public: bool apply(const SanitationServiceOp&); bool erase(std::uint64_t); const SanitationServiceData* find(std::uint64_t) const; std::vector<SanitationServiceData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SanitationServiceData> data_; };
}

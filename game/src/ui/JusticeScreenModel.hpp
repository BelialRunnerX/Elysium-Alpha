#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build incidents, laws, evidence, suspects, warrants, detention, trials, sentences, and security projections.
struct JusticeScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct JusticeScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class JusticeScreenModelStore { public: bool apply(const JusticeScreenModelOp&); bool erase(std::uint64_t); const JusticeScreenModelData* find(std::uint64_t) const; std::vector<JusticeScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,JusticeScreenModelData> data_; };
}

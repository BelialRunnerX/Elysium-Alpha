#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track captured personnel, exchange offers, diplomatic terms, verification, transport, and completion state.
struct PrisonerExchangeOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PrisonerExchangeData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PrisonerExchangeStore { public: bool apply(const PrisonerExchangeOp&); bool erase(std::uint64_t); const PrisonerExchangeData* find(std::uint64_t) const; std::vector<PrisonerExchangeData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PrisonerExchangeData> data_; };
}

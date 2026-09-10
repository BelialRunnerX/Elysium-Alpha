#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate deterministic faction founding, splits, wars, leaders, discoveries, migrations, and ideological shifts.
struct FactionHistoryGeneratorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FactionHistoryGeneratorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FactionHistoryGeneratorStore { public: bool apply(const FactionHistoryGeneratorOp&); bool erase(std::uint64_t); const FactionHistoryGeneratorData* find(std::uint64_t) const; std::vector<FactionHistoryGeneratorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FactionHistoryGeneratorData> data_; };
}

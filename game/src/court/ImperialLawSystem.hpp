#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track regionally scoped Imperial laws, decrees, exemptions, enforcement priority, conflicts, and repeal history.
struct ImperialLawSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ImperialLawSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ImperialLawSystemStore { public: bool apply(const ImperialLawSystemOp&); bool erase(std::uint64_t); const ImperialLawSystemData* find(std::uint64_t) const; std::vector<ImperialLawSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ImperialLawSystemData> data_; };
}

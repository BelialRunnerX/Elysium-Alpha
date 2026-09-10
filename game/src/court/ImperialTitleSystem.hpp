#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track titles, offices, honors, ranks, succession, privileges, obligations, and recognition by Imperial institutions.
struct ImperialTitleSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ImperialTitleSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ImperialTitleSystemStore { public: bool apply(const ImperialTitleSystemOp&); bool erase(std::uint64_t); const ImperialTitleSystemData* find(std::uint64_t) const; std::vector<ImperialTitleSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ImperialTitleSystemData> data_; };
}

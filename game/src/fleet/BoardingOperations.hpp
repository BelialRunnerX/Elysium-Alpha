#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track boarding teams, breach points, compartments, resistance, objectives, casualties, capture, and withdrawal.
struct BoardingOperationsOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BoardingOperationsData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BoardingOperationsStore { public: bool apply(const BoardingOperationsOp&); bool erase(std::uint64_t); const BoardingOperationsData* find(std::uint64_t) const; std::vector<BoardingOperationsData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BoardingOperationsData> data_; };
}

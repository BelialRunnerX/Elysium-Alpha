#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track detection, interpretation, communication, caution, diplomacy, misunderstanding, and Chronicle events for first contact.
struct FirstContactSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FirstContactSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FirstContactSystemStore { public: bool apply(const FirstContactSystemOp&); bool erase(std::uint64_t); const FirstContactSystemData* find(std::uint64_t) const; std::vector<FirstContactSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FirstContactSystemData> data_; };
}

#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent local shuttles, lifts, trams, buses, ferries, and passenger demand between stable settlement nodes.
struct PassengerTransitOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PassengerTransitData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PassengerTransitStore { public: bool apply(const PassengerTransitOp&); bool erase(std::uint64_t); const PassengerTransitData* find(std::uint64_t) const; std::vector<PassengerTransitData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PassengerTransitData> data_; };
}

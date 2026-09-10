#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track future multiplayer session identities, authentication state, shard presence, latency samples, and disconnect reasons.
struct ConnectionSessionModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ConnectionSessionModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ConnectionSessionModelStore { public: bool apply(const ConnectionSessionModelOp&); bool erase(std::uint64_t); const ConnectionSessionModelData* find(std::uint64_t) const; std::vector<ConnectionSessionModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ConnectionSessionModelData> data_; };
}

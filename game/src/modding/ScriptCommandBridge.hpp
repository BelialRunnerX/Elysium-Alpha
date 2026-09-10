#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Validate future script-issued intents into stable command envelopes before owner-system execution.
struct ScriptCommandBridgeOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ScriptCommandBridgeData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ScriptCommandBridgeStore { public: bool apply(const ScriptCommandBridgeOp&); bool erase(std::uint64_t); const ScriptCommandBridgeData* find(std::uint64_t) const; std::vector<ScriptCommandBridgeData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ScriptCommandBridgeData> data_; };
}

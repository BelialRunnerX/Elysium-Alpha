#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Expose controlled developer edit commands over stable world facts while keeping simulation stores authoritative.
struct WorldStateEditorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct WorldStateEditorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class WorldStateEditorStore { public: bool apply(const WorldStateEditorOp&); bool erase(std::uint64_t); const WorldStateEditorData* find(std::uint64_t) const; std::vector<WorldStateEditorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,WorldStateEditorData> data_; };
}

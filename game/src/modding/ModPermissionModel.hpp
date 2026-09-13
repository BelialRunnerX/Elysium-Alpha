#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent mod-declared capabilities for content, scripts, UI, files, networking, and unsafe/native extension boundaries.
struct ModPermissionModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ModPermissionModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ModPermissionModelStore { public: bool apply(const ModPermissionModelOp&); bool erase(std::uint64_t); const ModPermissionModelData* find(std::uint64_t) const; std::vector<ModPermissionModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ModPermissionModelData> data_; };
}

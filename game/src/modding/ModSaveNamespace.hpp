#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Provide stable per-mod save namespaces, schema versions, migrations, and orphaned-data preservation hooks.
struct ModSaveNamespaceOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ModSaveNamespaceData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ModSaveNamespaceStore { public: bool apply(const ModSaveNamespaceOp&); bool erase(std::uint64_t); const ModSaveNamespaceData* find(std::uint64_t) const; std::vector<ModSaveNamespaceData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ModSaveNamespaceData> data_; };
}

// Intended function: Resolve deterministic mod dependency graphs, versions, optional dependencies, load order, conflicts, and namespaces.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::mod {
struct ModDependencyState {
    std::uint64_t modId{};
    std::uint64_t version{};
    std::uint64_t dependencyHash{};
    std::uint64_t loadOrder{};
    std::uint64_t conflictMask{};
    std::uint64_t state{};
};
class ModDependencyStateCollection {
public:
 bool store(ModDependencyState value); bool erase(std::uint64_t id); [[nodiscard]] const ModDependencyState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ModDependencyState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ModDependencyState& v) noexcept; std::vector<ModDependencyState> rows_;
};
}

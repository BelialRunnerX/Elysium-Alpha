// Intended function: Define bounded mod permissions for data registration, presentation hooks, commands, file access, scripting, and multiplayer compatibility.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::mod {
struct ModPermissionState {
    std::uint64_t modId{};
    std::uint64_t permissionMask{};
    std::uint64_t sandboxLevel{};
    std::uint64_t networkPolicy{};
    std::uint64_t filePolicy{};
    std::uint64_t flags{};
};
class ModPermissionStateCollection {
public:
 bool store(ModPermissionState value); bool erase(std::uint64_t id); [[nodiscard]] const ModPermissionState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ModPermissionState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ModPermissionState& v) noexcept; std::vector<ModPermissionState> rows_;
};
}

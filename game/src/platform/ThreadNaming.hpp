// Intended function: Track worker/thread role labels and diagnostic identities without allowing thread identity to influence deterministic simulation.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::platform {
struct ThreadRole {
    std::uint64_t roleId{};
    std::uint64_t threadToken{};
    std::uint64_t workerIndex{};
    std::uint64_t systemId{};
    std::uint64_t flags{};
    std::uint64_t revision{};
};
class ThreadRoleCollection {
public:
 bool store(ThreadRole value); bool erase(std::uint64_t id); [[nodiscard]] const ThreadRole* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ThreadRole>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ThreadRole& v) noexcept; std::vector<ThreadRole> rows_;
};
}

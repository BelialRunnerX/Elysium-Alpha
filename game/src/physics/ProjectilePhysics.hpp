// Intended function: Track projectile position/velocity/gravity/drag/penetration/lifetime and deterministic collision sequence state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::physics {
struct ProjectilePhysicsState {
    std::uint64_t projectileId{};
    std::uint64_t positionHash{};
    std::uint64_t velocityHash{};
    double penetration{};
    double lifetime{};
    std::uint64_t flags{};
};
class ProjectilePhysicsStateIndex {
public:
 bool upsert(ProjectilePhysicsState value); bool erase(std::uint64_t id); [[nodiscard]] const ProjectilePhysicsState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ProjectilePhysicsState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const ProjectilePhysicsState& value) noexcept; std::vector<ProjectilePhysicsState> rows_;
};
}

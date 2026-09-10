// Intended function: Represent bounded presentation/physics debris chunks from structural collapse, mining, combat, and vehicle damage with lifetime caps.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::physics {
struct DebrisState {
    std::uint64_t debrisId{};
    std::uint64_t sourceId{};
    double mass{};
    std::uint64_t velocityHash{};
    double lifetime{};
    std::uint64_t flags{};
};
class DebrisStateIndex {
public:
 bool upsert(DebrisState value); bool erase(std::uint64_t id); [[nodiscard]] const DebrisState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<DebrisState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const DebrisState& value) noexcept; std::vector<DebrisState> rows_;
};
}

// Intended function: Project stable character appearance, armor layers, damage, equipment, species/body plan, cosmetics, and animation inputs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::render {
struct CharacterRenderState {
    std::uint64_t stableId{};
    std::uint64_t bodyPlanId{};
    std::uint64_t appearanceHash{};
    std::uint64_t equipmentHash{};
    std::uint64_t damageHash{};
    std::uint64_t flags{};
};
class CharacterRenderStateCollection {
public:
 bool store(CharacterRenderState value); bool erase(std::uint64_t id); [[nodiscard]] const CharacterRenderState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<CharacterRenderState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const CharacterRenderState& v) noexcept; std::vector<CharacterRenderState> rows_;
};
}

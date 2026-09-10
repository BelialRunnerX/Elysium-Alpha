// Intended function: Track contextual onboarding prompts for movement, mining, shelter, power, crafting, automation, combat, travel, and fortress commands.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::game {
struct TutorialState {
    std::uint64_t tutorialId{};
    std::uint64_t stepId{};
    double progress{};
    std::uint64_t completed{};
    std::uint64_t suppressed{};
    std::uint64_t flags{};
};
class TutorialStateCollection {
public:
 bool store(TutorialState value); bool erase(std::uint64_t id); [[nodiscard]] const TutorialState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<TutorialState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const TutorialState& v) noexcept; std::vector<TutorialState> rows_;
};
}

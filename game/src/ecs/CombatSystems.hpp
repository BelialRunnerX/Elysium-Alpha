// Intended function: Stage ECS combat intents, damage applications, deaths, effect commands, and deterministic structural commit requests.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct CombatCommand {
    std::uint64_t commandId{};
    std::uint64_t sourceId{};
    std::uint64_t targetId{};
    std::uint64_t kind{};
    double magnitude{};
    std::uint64_t sequence{};
};
class CombatCommandCollection {
public:
 bool store(CombatCommand value); bool erase(std::uint64_t id); [[nodiscard]] const CombatCommand* find(std::uint64_t id) const; [[nodiscard]] const std::vector<CombatCommand>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const CombatCommand& v) noexcept; std::vector<CombatCommand> rows_;
};
}

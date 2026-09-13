// Intended function: Stage ECS mission/objective progress, trigger, completion, failure, reward, and Chronicle publication commands.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct MissionCommand {
    std::uint64_t commandId{};
    std::uint64_t missionId{};
    std::uint64_t actorId{};
    std::uint64_t objectiveId{};
    std::uint64_t delta{};
    std::uint64_t sequence{};
};
class MissionCommandCollection {
public:
 bool store(MissionCommand value); bool erase(std::uint64_t id); [[nodiscard]] const MissionCommand* find(std::uint64_t id) const; [[nodiscard]] const std::vector<MissionCommand>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const MissionCommand& v) noexcept; std::vector<MissionCommand> rows_;
};
}

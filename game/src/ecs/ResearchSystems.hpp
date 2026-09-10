// Intended function: Stage ECS research/experiment/knowledge/discovery commands while keeping archives and registries stable-ID based.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct ResearchCommand {
    std::uint64_t commandId{};
    std::uint64_t researchId{};
    std::uint64_t actorId{};
    std::uint64_t sourceId{};
    double progress{};
    std::uint64_t sequence{};
};
class ResearchCommandCollection {
public:
 bool store(ResearchCommand value); bool erase(std::uint64_t id); [[nodiscard]] const ResearchCommand* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ResearchCommand>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ResearchCommand& v) noexcept; std::vector<ResearchCommand> rows_;
};
}

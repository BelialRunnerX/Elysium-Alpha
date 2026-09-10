// Intended function: Stage ECS citizen/job/room/stockpile/institution/governance/military commands for owner-thread structural commit.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct SettlementCommand {
    std::uint64_t commandId{};
    std::uint64_t siteId{};
    std::uint64_t actorId{};
    std::uint64_t targetId{};
    std::uint64_t kind{};
    std::uint64_t sequence{};
};
class SettlementCommandCollection {
public:
 bool store(SettlementCommand value); bool erase(std::uint64_t id); [[nodiscard]] const SettlementCommand* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SettlementCommand>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const SettlementCommand& v) noexcept; std::vector<SettlementCommand> rows_;
};
}

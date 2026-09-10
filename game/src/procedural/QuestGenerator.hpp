// Intended function: Generate deterministic contracts/missions from local needs, factions, threats, discoveries, economy, history, and player standing.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct QuestSeed {
    std::uint64_t questId{};
    std::uint64_t seed{};
    std::uint64_t issuerId{};
    std::uint64_t typeId{};
    std::uint64_t targetId{};
    double rewardBudget{};
};
class QuestSeedIndex {
public:
 bool upsert(QuestSeed value); bool erase(std::uint64_t id); [[nodiscard]] const QuestSeed* find(std::uint64_t id) const; [[nodiscard]] const std::vector<QuestSeed>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const QuestSeed& value) noexcept; std::vector<QuestSeed> rows_;
};
}

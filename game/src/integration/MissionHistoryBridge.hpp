// Intended function: Translate mission stage/completion/failure outcomes into Chronicle events and persistent objective consequences.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct MissionHistoryIntent {
    std::uint64_t intentId{};
    std::uint64_t missionId{};
    std::uint64_t stage{};
    std::uint64_t eventType{};
    double importance{};
    std::uint64_t tick{};
};
class MissionHistoryIntentIndex {
public:
 bool upsert(MissionHistoryIntent value); bool erase(std::uint64_t id); [[nodiscard]] const MissionHistoryIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<MissionHistoryIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const MissionHistoryIntent& value) noexcept; std::vector<MissionHistoryIntent> rows_;
};
}

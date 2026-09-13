// Intended function: Translate renderer-neutral tactical UI intents into stable squad/military command requests for authoritative commit.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct TacticalCommandIntent {
    std::uint64_t intentId{};
    std::uint64_t squadId{};
    std::uint64_t orderType{};
    std::uint64_t targetId{};
    std::uint64_t priority{};
    std::uint64_t flags{};
};
class TacticalCommandIntentIndex {
public:
 bool upsert(TacticalCommandIntent value); bool erase(std::uint64_t id); [[nodiscard]] const TacticalCommandIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<TacticalCommandIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const TacticalCommandIntent& value) noexcept; std::vector<TacticalCommandIntent> rows_;
};
}

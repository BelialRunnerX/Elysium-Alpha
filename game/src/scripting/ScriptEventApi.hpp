// Intended function: Expose read-only event subscriptions for combat, jobs, discoveries, trade, history, weather, machines, and player actions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::scripting {
struct ScriptSubscription {
    std::uint64_t subscriptionId{};
    std::uint64_t scriptId{};
    std::uint64_t eventMask{};
    std::uint64_t priority{};
    std::uint64_t filterHash{};
    std::uint64_t flags{};
};
class ScriptSubscriptionCollection {
public:
 bool store(ScriptSubscription value); bool erase(std::uint64_t id); [[nodiscard]] const ScriptSubscription* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ScriptSubscription>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ScriptSubscription& v) noexcept; std::vector<ScriptSubscription> rows_;
};
}

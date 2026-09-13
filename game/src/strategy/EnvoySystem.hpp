// Intended function: Represent faction/Court envoys, agendas, demands, gifts, negotiation state, hospitality, and diplomatic consequences.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::strategy {
struct EnvoyState {
    std::uint64_t envoyId{};
    std::uint64_t factionId{};
    std::uint64_t agendaId{};
    double standing{};
    double patience{};
    std::uint64_t state{};
};
class EnvoyStateStore {
public:
 bool put(EnvoyState v); bool erase(std::uint64_t id);
 [[nodiscard]] const EnvoyState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<EnvoyState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const EnvoyState& v) noexcept; std::vector<EnvoyState> values_;
};
} // namespace elysium::strategy

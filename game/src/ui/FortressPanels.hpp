// Intended function: Project citizen/jobs/stocks/zones/rooms/medicine/military/justice/governance/trade/history panels from authoritative snapshots.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct FortressPanelState {
    std::uint64_t panelId{};
    std::uint64_t category{};
    std::uint64_t rowCount{};
    std::uint64_t warningCount{};
    std::uint64_t criticalCount{};
    std::uint64_t revision{};
};
class FortressPanelStateCollection {
public:
 bool store(FortressPanelState value); bool erase(std::uint64_t id); [[nodiscard]] const FortressPanelState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<FortressPanelState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const FortressPanelState& v) noexcept; std::vector<FortressPanelState> rows_;
};
}

// Intended function: Project stable read-only blocker/reason/fact models for citizens, jobs, rooms, items, machines, military, trade, and history.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct InspectorPanel {
    std::uint64_t panelId{};
    std::uint64_t subjectId{};
    std::uint64_t category{};
    double severity{};
    std::uint64_t factHash{};
    std::uint64_t diagnosticHash{};
};
class InspectorPanelTable {
public:
 bool set(InspectorPanel value); bool remove(std::uint64_t id);
 [[nodiscard]] const InspectorPanel* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<InspectorPanel> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const InspectorPanel& value) noexcept; std::vector<InspectorPanel> rows_;
};
}

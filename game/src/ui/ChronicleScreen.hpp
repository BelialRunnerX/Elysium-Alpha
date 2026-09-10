// Intended function: Project searchable figures, sites, artifacts, battles, migrations, institutions, discoveries, and cross-linked event timelines.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct ChronicleViewState {
    std::uint64_t viewId{};
    std::uint64_t subjectId{};
    std::uint64_t subjectType{};
    std::uint64_t eventCount{};
    double importance{};
    std::uint64_t revision{};
};
class ChronicleViewStateTable {
public:
 bool set(ChronicleViewState value); bool remove(std::uint64_t id);
 [[nodiscard]] const ChronicleViewState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<ChronicleViewState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const ChronicleViewState& value) noexcept; std::vector<ChronicleViewState> rows_;
};
}

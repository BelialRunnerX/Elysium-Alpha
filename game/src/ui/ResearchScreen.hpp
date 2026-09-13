// Intended function: Project available research, prerequisites, experiments, specimens, apparatus, confidence, unlocks, and archive links.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct ResearchViewState {
    std::uint64_t viewId{};
    std::uint64_t researchId{};
    double progress{};
    double confidence{};
    std::uint64_t blockerCount{};
    std::uint64_t flags{};
};
class ResearchViewStateTable {
public:
 bool set(ResearchViewState value); bool remove(std::uint64_t id);
 [[nodiscard]] const ResearchViewState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<ResearchViewState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const ResearchViewState& value) noexcept; std::vector<ResearchViewState> rows_;
};
}

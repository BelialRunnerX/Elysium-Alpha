// Intended function: Project settlement population, jobs, stocks, power, atmosphere, food, medicine, security, alerts, trade, and history summaries.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct DashboardSection {
    std::uint64_t sectionId{};
    std::uint64_t category{};
    double value{};
    std::uint64_t warningCount{};
    std::uint64_t criticalCount{};
    std::uint64_t revision{};
};
class DashboardSectionTable {
public:
 bool set(DashboardSection value); bool remove(std::uint64_t id);
 [[nodiscard]] const DashboardSection* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<DashboardSection> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const DashboardSection& value) noexcept; std::vector<DashboardSection> rows_;
};
}

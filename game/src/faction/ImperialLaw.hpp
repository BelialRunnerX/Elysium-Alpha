// Intended function: Represent Imperial Code rules, registration requirements, contraband policy, inspection powers, warrants, and enforcement severity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::faction {
struct ImperialLawRecord {
    std::uint64_t lawId{};
    std::uint64_t category{};
    double severity{};
    double inspectionWeight{};
    double penalty{};
    std::uint64_t flags{};
};
class ImperialLawRecordTable {
public:
 bool set(ImperialLawRecord value); bool remove(std::uint64_t id);
 [[nodiscard]] const ImperialLawRecord* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<ImperialLawRecord> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const ImperialLawRecord& value) noexcept; std::vector<ImperialLawRecord> rows_;
};
}

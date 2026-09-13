// Intended function: Track Unsworn haven reputation, smuggling access, mutual-aid links, safehouses, favors, and suspicion-decay support.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::faction {
struct UnswornContact {
    std::uint64_t contactId{};
    std::uint64_t havenId{};
    double trust{};
    std::uint64_t serviceMask{};
    double risk{};
    std::uint64_t flags{};
};
class UnswornContactTable {
public:
 bool set(UnswornContact value); bool remove(std::uint64_t id);
 [[nodiscard]] const UnswornContact* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<UnswornContact> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const UnswornContact& value) noexcept; std::vector<UnswornContact> rows_;
};
}

// Intended function: Track normalized bilateral standing, treaties, hostilities, trade access, borders, guarantees, and historical grievance pressure.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::faction {
struct FactionRelation {
    std::uint64_t relationId{};
    std::uint64_t factionA{};
    std::uint64_t factionB{};
    double standing{};
    std::uint64_t treatyMask{};
    double grievance{};
};
class FactionRelationTable {
public:
 bool set(FactionRelation value); bool remove(std::uint64_t id);
 [[nodiscard]] const FactionRelation* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<FactionRelation> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const FactionRelation& value) noexcept; std::vector<FactionRelation> rows_;
};
}

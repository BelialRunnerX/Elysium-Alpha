// Intended function: Track stable item containers, capacities, filters, ownership, sealing, hazard policy, access, and nested-container restrictions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::inventory {
struct ContainerState {
    std::uint64_t containerId{};
    std::uint64_t ownerId{};
    double capacity{};
    double used{};
    std::uint64_t filterHash{};
    std::uint64_t flags{};
};
class ContainerStateIndex {
public:
 bool upsert(ContainerState value); bool erase(std::uint64_t id); [[nodiscard]] const ContainerState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ContainerState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const ContainerState& value) noexcept; std::vector<ContainerState> rows_;
};
}

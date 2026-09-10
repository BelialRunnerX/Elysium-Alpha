// Intended function: Route immutable simulation events by stable subject/source/category without granting subscribers mutation authority.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::simulation {
struct SimEvent {
    std::uint64_t eventId{};
    std::uint64_t category{};
    std::uint64_t sourceId{};
    std::uint64_t subjectId{};
    std::uint64_t tick{};
    std::uint64_t flags{};
};
class SimEventTable {
public:
 bool set(SimEvent value); bool remove(std::uint64_t id);
 [[nodiscard]] const SimEvent* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<SimEvent> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const SimEvent& value) noexcept; std::vector<SimEvent> rows_;
};
}

// Intended function: Route committed construction/destruction invalidation to navigation, rooms, atmosphere, utilities, supports, and renderer rebuild queues.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct InvalidationIntent {
    std::uint64_t intentId{};
    std::uint64_t addressKey{};
    std::uint64_t domainMask{};
    std::uint64_t worldRevision{};
    std::uint64_t priority{};
    std::uint64_t flags{};
};
class InvalidationIntentIndex {
public:
 bool upsert(InvalidationIntent value); bool erase(std::uint64_t id); [[nodiscard]] const InvalidationIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<InvalidationIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const InvalidationIntent& value) noexcept; std::vector<InvalidationIntent> rows_;
};
}

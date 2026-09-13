// Intended function: Apply deterministically ordered stable commands during Commit phase and record rejected conflicts/reasons.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct CommitRecord {
    std::uint64_t recordId{};
    std::uint64_t commandId{};
    std::uint64_t stableKey{};
    std::uint64_t kind{};
    std::uint64_t result{};
    std::uint64_t conflictKey{};
};
class CommitRecordCollection {
public:
 bool store(CommitRecord value); bool erase(std::uint64_t id); [[nodiscard]] const CommitRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<CommitRecord>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const CommitRecord& v) noexcept; std::vector<CommitRecord> rows_;
};
}

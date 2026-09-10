// Intended function: Track future local prediction sequence, acknowledged commands, input windows, correction pressure, and simulation tick.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::network {
struct PredictionStateRecord {
    std::uint64_t actorId{};
    std::uint64_t localSequence{};
    std::uint64_t ackSequence{};
    std::uint64_t tick{};
    double correction{};
    std::uint64_t flags{};
};
class PredictionStateRecordCollection {
public:
 bool store(PredictionStateRecord value); bool erase(std::uint64_t id); [[nodiscard]] const PredictionStateRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<PredictionStateRecord>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const PredictionStateRecord& v) noexcept; std::vector<PredictionStateRecord> rows_;
};
}

// Intended function: Record deterministic gameplay commands, world revisions, seeds, checkpoints, and replay cursors for debugging and demos.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::replay {
struct ReplayFrame {
    std::uint64_t frameId{};
    std::uint64_t tick{};
    std::uint64_t commandHash{};
    std::uint64_t worldRevision{};
    std::uint64_t seedHash{};
    std::uint64_t flags{};
};
class ReplayFrameStore {
public:
 bool put(ReplayFrame v); bool erase(std::uint64_t id);
 [[nodiscard]] const ReplayFrame* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<ReplayFrame>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const ReplayFrame& v) noexcept; std::vector<ReplayFrame> values_;
};
} // namespace elysium::replay

// Intended function: Track persistent operative experience, levels, unlock points, milestone flags, and deterministic progression rewards.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::gameplay {

struct ProgressionTrack {
    std::uint64_t trackId{};
    std::uint64_t level{};
    double experience{};
    std::uint64_t unspentPoints{};
    std::uint64_t milestoneMask{};
    std::uint64_t revision{};
};

class ProgressionTrackStore {
public:
    bool upsert(ProgressionTrack value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const ProgressionTrack* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<ProgressionTrack> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const ProgressionTrack& value) noexcept;
    std::vector<ProgressionTrack> records_;
};

} // namespace elysium::gameplay

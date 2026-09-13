// Intended function: Select adaptive music states from exploration, combat, fortress crises, Imperial pressure, Rift threat, and story beats.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::audio {
struct MusicState {
    std::uint64_t stateId{};
    std::uint64_t cueId{};
    double intensity{};
    std::uint64_t priority{};
    std::uint64_t transitionTicks{};
    std::uint64_t flags{};
};
class MusicStateRegistry {
public:
    bool publish(MusicState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const MusicState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<MusicState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const MusicState& r) noexcept;
    std::vector<MusicState> records_;
};
} // namespace elysium::audio

// Intended function: Project pressure, smoke, fire, contamination, precipitation, fog, and decompression into bounded visual effects.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::render {
struct AtmosphereVisualState {
    double volumeId{};
    double pressure{};
    double smoke{};
    double fire{};
    double fog{};
    std::uint64_t effectFlags{};
};
class AtmosphereVisualStateRegistry {
public:
    bool publish(AtmosphereVisualState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const AtmosphereVisualState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<AtmosphereVisualState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const AtmosphereVisualState& r) noexcept;
    std::vector<AtmosphereVisualState> records_;
};
} // namespace elysium::render

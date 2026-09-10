// Intended function: Select layered ambient loops from biome, weather, machinery, settlement activity, danger, and interior/exterior context.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::audio {
struct AmbienceLayer {
    std::uint64_t layerId{};
    std::uint64_t contextId{};
    double gain{};
    std::uint64_t priority{};
    std::uint64_t fadeTicks{};
    std::uint64_t flags{};
};
class AmbienceLayerRegistry {
public:
    bool publish(AmbienceLayer record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const AmbienceLayer* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<AmbienceLayer>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const AmbienceLayer& r) noexcept;
    std::vector<AmbienceLayer> records_;
};
} // namespace elysium::audio

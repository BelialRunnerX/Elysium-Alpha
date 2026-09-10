// Intended function: Queue bounded presentation-only particle events from combat, mining, weather, machines, damage, and atmosphere.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::render {
struct ParticleEvent {
    std::uint64_t eventId{};
    std::uint64_t kind{};
    std::uint64_t sourceId{};
    double magnitude{};
    double lifetime{};
    std::uint64_t flags{};
};
class ParticleEventRegistry {
public:
    bool publish(ParticleEvent record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ParticleEvent* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ParticleEvent>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ParticleEvent& r) noexcept;
    std::vector<ParticleEvent> records_;
};
} // namespace elysium::render

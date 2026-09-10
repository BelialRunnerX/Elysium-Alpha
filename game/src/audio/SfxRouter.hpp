// Intended function: Route presentation events into bounded sound-effect requests by material, weapon, machine, UI, weather, creature, and spatial context.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::audio {
struct SfxEvent {
    std::uint64_t eventId{};
    std::uint64_t cueId{};
    std::uint64_t sourceId{};
    double gain{};
    double pitch{};
    std::uint64_t flags{};
};
class SfxEventCollection {
public:
 bool store(SfxEvent value); bool erase(std::uint64_t id); [[nodiscard]] const SfxEvent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SfxEvent>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const SfxEvent& v) noexcept; std::vector<SfxEvent> rows_;
};
}

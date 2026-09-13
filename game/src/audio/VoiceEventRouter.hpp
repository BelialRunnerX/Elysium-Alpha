// Intended function: Route dialogue/combat/alert/effort voice events by speaker profile, priority, cooldown, subtitle key, and localization state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::audio {
struct VoiceEvent {
    std::uint64_t eventId{};
    std::uint64_t speakerId{};
    std::uint64_t cueId{};
    std::uint64_t priority{};
    double cooldown{};
    std::uint64_t subtitleKey{};
};
class VoiceEventCollection {
public:
 bool store(VoiceEvent value); bool erase(std::uint64_t id); [[nodiscard]] const VoiceEvent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<VoiceEvent>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const VoiceEvent& v) noexcept; std::vector<VoiceEvent> rows_;
};
}

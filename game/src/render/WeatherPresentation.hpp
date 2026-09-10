// Intended function: Project weather into sky, fog, precipitation, wind effects, lighting, wetness, particles, and audio cue parameters.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::render {
struct WeatherRenderState {
    std::uint64_t weatherId{};
    std::uint64_t skyState{};
    double fog{};
    double precipitation{};
    double wind{};
    std::uint64_t effectMask{};
};
class WeatherRenderStateCollection {
public:
 bool store(WeatherRenderState value); bool erase(std::uint64_t id); [[nodiscard]] const WeatherRenderState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<WeatherRenderState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const WeatherRenderState& v) noexcept; std::vector<WeatherRenderState> rows_;
};
}

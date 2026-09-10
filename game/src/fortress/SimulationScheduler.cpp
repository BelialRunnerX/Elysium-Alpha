#include "fortress/SimulationScheduler.hpp"

#include <algorithm>

namespace elysium::fortress {
namespace {
constexpr std::size_t indexOf(ClockFamily family) { return static_cast<std::size_t>(family); }
}

SimulationScheduler::SimulationScheduler() {
    clocks_[indexOf(ClockFamily::Frame)].intervalSeconds = 1.0 / 60.0;
    clocks_[indexOf(ClockFamily::Tactical)].intervalSeconds = 1.0 / 30.0;
    clocks_[indexOf(ClockFamily::LocalSimulation)].intervalSeconds = 1.0 / 8.0;
    clocks_[indexOf(ClockFamily::Citizen)].intervalSeconds = 1.0;
    clocks_[indexOf(ClockFamily::FortressEconomy)].intervalSeconds = 2.0;
    clocks_[indexOf(ClockFamily::Ecology)].intervalSeconds = 60.0;
    clocks_[indexOf(ClockFamily::Strategic)].intervalSeconds = 3600.0;
    clocks_[indexOf(ClockFamily::Historical)].intervalSeconds = 86400.0;
}

std::vector<ClockPulse> SimulationScheduler::advance(double realSeconds, double timeScale) {
    std::vector<ClockPulse> pulses;
    const double scaled = std::max(0.0, realSeconds) * std::max(0.0, timeScale);
    for (std::size_t i = 0; i < clocks_.size(); ++i) {
        auto& clock = clocks_[i];
        clock.accumulator += scaled;
        const double interval = std::max(1e-6, clock.intervalSeconds);
        std::uint32_t guard{};
        while (clock.accumulator >= interval && guard < 4096) {
            clock.accumulator -= interval;
            ++clock.epochs;
            pulses.push_back(ClockPulse{static_cast<ClockFamily>(i), clock.epochs, interval});
            ++guard;
        }
    }
    return pulses;
}

const ClockState& SimulationScheduler::clock(ClockFamily family) const {
    return clocks_[indexOf(family)];
}

void SimulationScheduler::setInterval(ClockFamily family, double seconds) {
    clocks_[indexOf(family)].intervalSeconds = std::max(1e-6, seconds);
}

} // namespace elysium::fortress

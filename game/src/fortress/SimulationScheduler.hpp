#pragma once

#include "fortress/Common.hpp"

#include <array>
#include <vector>

namespace elysium::fortress {

struct ClockPulse {
    ClockFamily family{ClockFamily::Frame};
    std::uint64_t epoch{};
    double dt{};
};

class SimulationScheduler {
public:
    SimulationScheduler();
    std::vector<ClockPulse> advance(double realSeconds, double timeScale = 1.0);
    const ClockState& clock(ClockFamily family) const;
    void setInterval(ClockFamily family, double seconds);

private:
    std::array<ClockState, 8> clocks_{};
};

} // namespace elysium::fortress

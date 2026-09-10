#pragma once

#include "fortress/Components.hpp"

namespace elysium::fortress {

enum class MechanismTriggerKind : std::uint8_t {
    Manual,
    Presence,
    Pressure,
    FluidLevel,
    Oxygen,
    ToxicGas,
    Thermal,
    Smoke,
    Inventory,
    Power,
    Security,
    Empire,
    Schedule
};

struct MechanismState {
    StableId id{};
    MechanismTriggerKind trigger{MechanismTriggerKind::Manual};
    StableId target{};
    ContentId action;
    float threshold{};
    float hysteresis{0.05f};
    bool state{};
    bool enabled{true};
};

bool updateMechanism(MechanismState& mechanism, float signal, bool manualSignal = false);

} // namespace elysium::fortress

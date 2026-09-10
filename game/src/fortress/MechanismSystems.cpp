#include "fortress/MechanismSystems.hpp"

namespace elysium::fortress {

bool updateMechanism(MechanismState& mechanism, float signal, bool manualSignal) {
    if (!mechanism.enabled) return false;
    const bool previous = mechanism.state;
    if (mechanism.trigger == MechanismTriggerKind::Manual) {
        mechanism.state = manualSignal;
    } else if (mechanism.state) {
        mechanism.state = signal >= mechanism.threshold - mechanism.hysteresis;
    } else {
        mechanism.state = signal >= mechanism.threshold + mechanism.hysteresis;
    }
    return previous != mechanism.state;
}

} // namespace elysium::fortress

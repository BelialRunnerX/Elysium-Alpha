#include "fortress/Systems.hpp"

#include <algorithm>

namespace elysium::fortress {

InspectorTrace explainStress(StableId citizen, const StressState& stress, const Needs& needs,
                             const Memories& memories) {
    InspectorTrace trace{};
    trace.subject = citizen;
    trace.decision = "stress band " + std::to_string(static_cast<unsigned>(stress.band));
    for (const auto& need : needs.states) {
        if (need.urgency < 0.15f) continue;
        trace.facts.push_back(InspectorFact{"need", "need #" + std::to_string(static_cast<unsigned>(need.kind)), need.urgency, citizen});
    }
    for (const auto& memory : memories.entries) {
        const float weight = std::max(0.0f, -memory.valence) * memory.strength * (1.0f + memory.trauma);
        if (weight > 0.12f) trace.facts.push_back(InspectorFact{"memory", memory.key, weight, memory.subject});
    }
    for (const auto& reason : stress.topReasons) trace.facts.push_back(InspectorFact{"system", reason, stress.load, citizen});
    std::sort(trace.facts.begin(), trace.facts.end(), [](const InspectorFact& a, const InspectorFact& b) { return a.weight > b.weight; });
    return trace;
}

InspectorTrace explainJob(const JobComponent& job) {
    InspectorTrace trace{};
    trace.subject = StableId{job.id.value};
    trace.decision = "job state " + std::to_string(static_cast<unsigned>(job.state));
    trace.facts.push_back(InspectorFact{"job", job.type.value, 1.0f, job.origin});
    trace.facts.push_back(InspectorFact{"priority", std::to_string(static_cast<unsigned>(job.priority)), 0.8f, {}});
    if (!job.failureReason.empty()) trace.facts.push_back(InspectorFact{"blocker", job.failureReason, 1.0f, {}});
    if (job.worker) trace.facts.push_back(InspectorFact{"worker", "claimed", 0.7f, job.worker});
    for (const auto dep : job.dependencies) trace.facts.push_back(InspectorFact{"dependency", "waits for job", 0.5f, StableId{dep.value}});
    for (const auto& reservation : job.reservations) trace.facts.push_back(InspectorFact{"reservation", std::to_string(static_cast<unsigned>(reservation.kind)), 0.4f, reservation.resource});
    return trace;
}

InspectorTrace explainMachine(const MachineState& machine, const MaintenanceState& maintenance) {
    InspectorTrace trace{};
    trace.subject = machine.id;
    trace.decision = machine.faulted ? "faulted" : machine.powered ? "operating" : "not powered";
    trace.facts.push_back(InspectorFact{"power", machine.powered ? "served" : "unserved", machine.powerDraw, machine.id});
    trace.facts.push_back(InspectorFact{"condition", "machine condition", 1.0f - machine.condition, machine.id});
    trace.facts.push_back(InspectorFact{"maintenance", maintenance.serviceRequested ? "service requested" : "service nominal", 1.0f - maintenance.condition, machine.id});
    if (machine.contamination > 0.0f) trace.facts.push_back(InspectorFact{"contamination", "contaminated", machine.contamination, machine.id});
    if (machine.heat > 0.7f) trace.facts.push_back(InspectorFact{"thermal", "high heat", machine.heat, machine.id});
    return trace;
}

std::vector<AlertRecord> collectAlerts(SiteId site,
                                       std::span<const ThreatState> threats,
                                       std::span<const MachineState> machines,
                                       std::span<const StressState> populationStress,
                                       std::span<const RoomAtmosphere> rooms) {
    std::vector<AlertRecord> alerts;
    for (const auto& threat : threats) {
        if (!threat.active || threat.target != site) continue;
        alerts.push_back(AlertRecord{threat.alert, "threat", "Threat active", "strength " + std::to_string(threat.strength), threat.id, site, threat.strength});
    }
    for (const auto& machine : machines) {
        if (machine.faulted || machine.condition < 0.25f) alerts.push_back(AlertRecord{machine.faulted ? AlertLevel::Red : AlertLevel::Yellow, "machine", "Machine maintenance", machine.type.value, machine.id, site, 1.0f - machine.condition});
    }
    std::size_t crisisCount{};
    for (const auto& stress : populationStress) if (stress.band == StressBand::Crisis) ++crisisCount;
    if (crisisCount > 0) alerts.push_back(AlertRecord{crisisCount > 3 ? AlertLevel::Red : AlertLevel::Yellow, "population", "Citizen stress crises", std::to_string(crisisCount) + " citizens in crisis", {}, site, static_cast<float>(crisisCount)});
    for (const auto& room : rooms) {
        if (room.room.value == 0) continue;
        if (room.sealed && (room.pressure < 0.5f || room.oxygen < 0.12f)) alerts.push_back(AlertRecord{AlertLevel::Red, "atmosphere", "Habitable volume compromised", "pressure/oxygen below safe threshold", StableId{room.room.value}, site, 1.0f});
        if (room.smoke > 0.35f || room.toxins > 0.25f) alerts.push_back(AlertRecord{AlertLevel::Red, "environment", "Air contamination", "smoke/toxin concentration elevated", StableId{room.room.value}, site, std::max(room.smoke, room.toxins)});
    }
    std::stable_sort(alerts.begin(), alerts.end(), [](const AlertRecord& a, const AlertRecord& b) {
        if (a.level != b.level) return static_cast<unsigned>(a.level) > static_cast<unsigned>(b.level);
        return a.urgency > b.urgency;
    });
    return alerts;
}

} // namespace elysium::fortress

#include "devtools/SimulationRecorder.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <tuple>

namespace elysium::devtools {

bool SimulationRecorder::record(SimulationRecord record) {
    if (!record.stableId) return false;
    sim_.push_back(record);
    return true;
}

bool SimulationRecorder::graph(GraphRecord record) {
    if (!record.node || !record.network) return false;
    graphs_.push_back(record);
    return true;
}

bool SimulationRecorder::performance(PerformanceRecord record) {
    if (!std::isfinite(record.value) || record.value < 0.0) record.value = 0.0;
    perf_.push_back(record);
    return true;
}

std::string SimulationRecorder::report() const {
    auto sim = sim_;
    auto graphs = graphs_;
    auto perf = perf_;
    std::sort(sim.begin(), sim.end(), [](const auto& a, const auto& b) {
        return std::tie(a.tick, a.stableId, a.componentOrEvent) < std::tie(b.tick, b.stableId, b.componentOrEvent);
    });
    std::sort(graphs.begin(), graphs.end(), [](const auto& a, const auto& b) {
        return std::tie(a.network, a.node) < std::tie(b.network, b.node);
    });
    std::sort(perf.begin(), perf.end(), [](const auto& a, const auto& b) {
        return std::tie(a.tick, a.metric) < std::tie(b.tick, b.metric);
    });

    std::ostringstream out;
    for (const auto& row : sim) {
        out << "SIM," << row.tick << ',' << row.stableId << ',' << row.componentOrEvent << ',' << row.stateHash << ',' << row.outcome << '\n';
    }
    for (const auto& row : graphs) {
        out << "GRAPH," << row.network << ',' << row.node << ',' << row.supply << ',' << row.demand << ',' << row.throughput << ',' << row.blockerCode << '\n';
    }
    for (const auto& row : perf) {
        out << "PERF," << row.tick << ',' << row.metric << ',' << row.value << ',' << unsigned(row.evidenceClass) << '\n';
    }
    return out.str();
}

} // namespace elysium::devtools

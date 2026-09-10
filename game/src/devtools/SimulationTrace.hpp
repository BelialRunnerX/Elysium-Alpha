// Intended function: Record compact deterministic simulation phase and command traces for offline diagnosis.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct SimulationTraceRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SimulationTraceRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SimulationTraceNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SimulationTraceSystem {
public:
 bool submit(const SimulationTraceRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SimulationTraceRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SimulationTraceRecord> snapshot() const;
 std::vector<SimulationTraceNotice> drainNotices(); void clear();
private:
 SimulationTraceRecord* mutableFind(std::uint64_t); void notice(const SimulationTraceRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SimulationTraceRecord> records_; std::vector<SimulationTraceNotice> notices_;
};
}

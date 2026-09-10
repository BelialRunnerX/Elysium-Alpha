// Intended function: Aggregate stalled jobs, invariant warnings, queue growth, budget pressure, and recovery actions.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct SimulationHealthRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SimulationHealthRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SimulationHealthNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SimulationHealthSystem {
public:
 bool submit(const SimulationHealthRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SimulationHealthRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SimulationHealthRecord> snapshot() const;
 std::vector<SimulationHealthNotice> drainNotices(); void clear();
private:
 SimulationHealthRecord* mutableFind(std::uint64_t); void notice(const SimulationHealthRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SimulationHealthRecord> records_; std::vector<SimulationHealthNotice> notices_;
};
}

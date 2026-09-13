// Intended function: Track overloaded systems and reduce optional work without changing authoritative results.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct SimulationBackpressureRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SimulationBackpressureRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SimulationBackpressureNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SimulationBackpressureSystem {
public:
 bool submit(const SimulationBackpressureRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SimulationBackpressureRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SimulationBackpressureRecord> snapshot() const;
 std::vector<SimulationBackpressureNotice> drainNotices(); void clear();
private:
 SimulationBackpressureRecord* mutableFind(std::uint64_t); void notice(const SimulationBackpressureRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SimulationBackpressureRecord> records_; std::vector<SimulationBackpressureNotice> notices_;
};
}

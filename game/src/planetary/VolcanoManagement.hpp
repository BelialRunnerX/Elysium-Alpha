// Intended function: Track magma pressure, vents, diversion, cooling, evacuation, and eruption risk.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct VolcanoManagementRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct VolcanoManagementRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct VolcanoManagementNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class VolcanoManagementSystem {
public:
 bool submit(const VolcanoManagementRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const VolcanoManagementRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<VolcanoManagementRecord> snapshot() const;
 std::vector<VolcanoManagementNotice> drainNotices(); void clear();
private:
 VolcanoManagementRecord* mutableFind(std::uint64_t); void notice(const VolcanoManagementRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<VolcanoManagementRecord> records_; std::vector<VolcanoManagementNotice> notices_;
};
}

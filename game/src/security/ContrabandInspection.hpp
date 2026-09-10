// Intended function: Track inspections, manifests, concealment, evidence, seizure, and suspicion changes.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct ContrabandInspectionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ContrabandInspectionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ContrabandInspectionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ContrabandInspectionSystem {
public:
 bool submit(const ContrabandInspectionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ContrabandInspectionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ContrabandInspectionRecord> snapshot() const;
 std::vector<ContrabandInspectionNotice> drainNotices(); void clear();
private:
 ContrabandInspectionRecord* mutableFind(std::uint64_t); void notice(const ContrabandInspectionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ContrabandInspectionRecord> records_; std::vector<ContrabandInspectionNotice> notices_;
};
}

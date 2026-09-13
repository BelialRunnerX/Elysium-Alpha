// Intended function: Track patrol routes, posts, threat coverage, incidents, and reassignment.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct SecurityPatrolPlannerRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SecurityPatrolPlannerRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SecurityPatrolPlannerNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SecurityPatrolPlannerSystem {
public:
 bool submit(const SecurityPatrolPlannerRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SecurityPatrolPlannerRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SecurityPatrolPlannerRecord> snapshot() const;
 std::vector<SecurityPatrolPlannerNotice> drainNotices(); void clear();
private:
 SecurityPatrolPlannerRecord* mutableFind(std::uint64_t); void notice(const SecurityPatrolPlannerRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SecurityPatrolPlannerRecord> records_; std::vector<SecurityPatrolPlannerNotice> notices_;
};
}

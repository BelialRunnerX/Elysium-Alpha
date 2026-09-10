// Intended function: Share bounded planner facts, reservations, goals, and coordination state among cooperating AI.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct PlanBlackboardRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct PlanBlackboardRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct PlanBlackboardNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class PlanBlackboardSystem {
public:
 bool submit(const PlanBlackboardRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const PlanBlackboardRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<PlanBlackboardRecord> snapshot() const;
 std::vector<PlanBlackboardNotice> drainNotices(); void clear();
private:
 PlanBlackboardRecord* mutableFind(std::uint64_t); void notice(const PlanBlackboardRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<PlanBlackboardRecord> records_; std::vector<PlanBlackboardNotice> notices_;
};
}

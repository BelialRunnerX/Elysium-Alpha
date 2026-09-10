// Intended function: Schedule strategic daily/monthly/yearly simulation work with deterministic bounded catch-up.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct LongHorizonSchedulerRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct LongHorizonSchedulerRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct LongHorizonSchedulerNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class LongHorizonSchedulerSystem {
public:
 bool submit(const LongHorizonSchedulerRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const LongHorizonSchedulerRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<LongHorizonSchedulerRecord> snapshot() const;
 std::vector<LongHorizonSchedulerNotice> drainNotices(); void clear();
private:
 LongHorizonSchedulerRecord* mutableFind(std::uint64_t); void notice(const LongHorizonSchedulerRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<LongHorizonSchedulerRecord> records_; std::vector<LongHorizonSchedulerNotice> notices_;
};
}

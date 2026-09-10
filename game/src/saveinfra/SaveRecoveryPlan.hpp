// Intended function: Select known-good generations and recovery paths after torn or corrupt transactions.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct SaveRecoveryPlanRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SaveRecoveryPlanRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SaveRecoveryPlanNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SaveRecoveryPlanSystem {
public:
 bool submit(const SaveRecoveryPlanRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SaveRecoveryPlanRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SaveRecoveryPlanRecord> snapshot() const;
 std::vector<SaveRecoveryPlanNotice> drainNotices(); void clear();
private:
 SaveRecoveryPlanRecord* mutableFind(std::uint64_t); void notice(const SaveRecoveryPlanRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SaveRecoveryPlanRecord> records_; std::vector<SaveRecoveryPlanNotice> notices_;
};
}

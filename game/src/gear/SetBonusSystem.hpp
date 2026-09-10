// Intended function: Track equipped item sets, thresholds, active bonuses, and explainable contribution breakdowns.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct SetBonusSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SetBonusSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SetBonusSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SetBonusSystemSystem {
public:
 bool submit(const SetBonusSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SetBonusSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SetBonusSystemRecord> snapshot() const;
 std::vector<SetBonusSystemNotice> drainNotices(); void clear();
private:
 SetBonusSystemRecord* mutableFind(std::uint64_t); void notice(const SetBonusSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SetBonusSystemRecord> records_; std::vector<SetBonusSystemNotice> notices_;
};
}

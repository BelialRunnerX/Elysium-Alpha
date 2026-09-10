// Intended function: Track contribution, objective participation, difficulty, loot rolls, and progression rewards.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::combatx {
struct CombatRewardSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CombatRewardSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CombatRewardSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CombatRewardSystemSystem {
public:
 bool submit(const CombatRewardSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CombatRewardSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CombatRewardSystemRecord> snapshot() const;
 std::vector<CombatRewardSystemNotice> drainNotices(); void clear();
private:
 CombatRewardSystemRecord* mutableFind(std::uint64_t); void notice(const CombatRewardSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CombatRewardSystemRecord> records_; std::vector<CombatRewardSystemNotice> notices_;
};
}

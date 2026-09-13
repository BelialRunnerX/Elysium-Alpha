// Intended function: Track ability activation, costs, cooldowns, targeting, interruption, and resolution.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::combatx {
struct CombatAbilitySystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CombatAbilitySystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CombatAbilitySystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CombatAbilitySystemSystem {
public:
 bool submit(const CombatAbilitySystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CombatAbilitySystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CombatAbilitySystemRecord> snapshot() const;
 std::vector<CombatAbilitySystemNotice> drainNotices(); void clear();
private:
 CombatAbilitySystemRecord* mutableFind(std::uint64_t); void notice(const CombatAbilitySystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CombatAbilitySystemRecord> records_; std::vector<CombatAbilitySystemNotice> notices_;
};
}

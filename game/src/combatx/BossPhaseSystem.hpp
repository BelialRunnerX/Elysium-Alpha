// Intended function: Track multi-stage elite/boss encounters with phase triggers and systemic state changes.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::combatx {
struct BossPhaseSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct BossPhaseSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct BossPhaseSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class BossPhaseSystemSystem {
public:
 bool submit(const BossPhaseSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const BossPhaseSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<BossPhaseSystemRecord> snapshot() const;
 std::vector<BossPhaseSystemNotice> drainNotices(); void clear();
private:
 BossPhaseSystemRecord* mutableFind(std::uint64_t); void notice(const BossPhaseSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<BossPhaseSystemRecord> records_; std::vector<BossPhaseSystemNotice> notices_;
};
}

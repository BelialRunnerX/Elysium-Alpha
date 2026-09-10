// Intended function: Track weapon combo chains, timing windows, stamina, stance, and contextual finishers.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::combatx {
struct MeleeComboSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct MeleeComboSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct MeleeComboSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class MeleeComboSystemSystem {
public:
 bool submit(const MeleeComboSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const MeleeComboSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<MeleeComboSystemRecord> snapshot() const;
 std::vector<MeleeComboSystemNotice> drainNotices(); void clear();
private:
 MeleeComboSystemRecord* mutableFind(std::uint64_t); void notice(const MeleeComboSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<MeleeComboSystemRecord> records_; std::vector<MeleeComboSystemNotice> notices_;
};
}

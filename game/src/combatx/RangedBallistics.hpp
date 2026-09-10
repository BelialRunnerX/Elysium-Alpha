// Intended function: Track projectile launch solutions, gravity, drag abstraction, penetration, and impact context.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::combatx {
struct RangedBallisticsRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct RangedBallisticsRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct RangedBallisticsNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class RangedBallisticsSystem {
public:
 bool submit(const RangedBallisticsRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const RangedBallisticsRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<RangedBallisticsRecord> snapshot() const;
 std::vector<RangedBallisticsNotice> drainNotices(); void clear();
private:
 RangedBallisticsRecord* mutableFind(std::uint64_t); void notice(const RangedBallisticsRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<RangedBallisticsRecord> records_; std::vector<RangedBallisticsNotice> notices_;
};
}

// Intended function: Resolve layered armor penetration, deflection, spall, residual energy, and localized damage.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::combatx {
struct ArmorPenetrationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ArmorPenetrationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ArmorPenetrationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ArmorPenetrationSystem {
public:
 bool submit(const ArmorPenetrationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ArmorPenetrationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ArmorPenetrationRecord> snapshot() const;
 std::vector<ArmorPenetrationNotice> drainNotices(); void clear();
private:
 ArmorPenetrationRecord* mutableFind(std::uint64_t); void notice(const ArmorPenetrationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ArmorPenetrationRecord> records_; std::vector<ArmorPenetrationNotice> notices_;
};
}

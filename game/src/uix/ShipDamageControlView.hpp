// Intended function: Project ship compartments, hull, fires, atmosphere, power, repairs, and priorities.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct ShipDamageControlViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipDamageControlViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipDamageControlViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipDamageControlViewSystem {
public:
 bool submit(const ShipDamageControlViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipDamageControlViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipDamageControlViewRecord> snapshot() const;
 std::vector<ShipDamageControlViewNotice> drainNotices(); void clear();
private:
 ShipDamageControlViewRecord* mutableFind(std::uint64_t); void notice(const ShipDamageControlViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipDamageControlViewRecord> records_; std::vector<ShipDamageControlViewNotice> notices_;
};
}

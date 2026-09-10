// Intended function: Track generation, buses, breakers, critical loads, overloads, and emergency shedding.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipPowerGridRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipPowerGridRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipPowerGridNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipPowerGridSystem {
public:
 bool submit(const ShipPowerGridRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipPowerGridRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipPowerGridRecord> snapshot() const;
 std::vector<ShipPowerGridNotice> drainNotices(); void clear();
private:
 ShipPowerGridRecord* mutableFind(std::uint64_t); void notice(const ShipPowerGridRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipPowerGridRecord> records_; std::vector<ShipPowerGridNotice> notices_;
};
}

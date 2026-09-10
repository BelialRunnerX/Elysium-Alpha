// Intended function: Track heat sources, radiators, coolant loops, thermal reserves, and overheat risk.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipThermalControlRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipThermalControlRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipThermalControlNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipThermalControlSystem {
public:
 bool submit(const ShipThermalControlRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipThermalControlRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipThermalControlRecord> snapshot() const;
 std::vector<ShipThermalControlNotice> drainNotices(); void clear();
private:
 ShipThermalControlRecord* mutableFind(std::uint64_t); void notice(const ShipThermalControlRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipThermalControlRecord> records_; std::vector<ShipThermalControlNotice> notices_;
};
}

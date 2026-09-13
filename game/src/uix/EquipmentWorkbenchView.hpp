// Intended function: Project gear stats, durability, sockets, upgrades, repair, comparisons, and provenance.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct EquipmentWorkbenchViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct EquipmentWorkbenchViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct EquipmentWorkbenchViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class EquipmentWorkbenchViewSystem {
public:
 bool submit(const EquipmentWorkbenchViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const EquipmentWorkbenchViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<EquipmentWorkbenchViewRecord> snapshot() const;
 std::vector<EquipmentWorkbenchViewNotice> drainNotices(); void clear();
private:
 EquipmentWorkbenchViewRecord* mutableFind(std::uint64_t); void notice(const EquipmentWorkbenchViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<EquipmentWorkbenchViewRecord> records_; std::vector<EquipmentWorkbenchViewNotice> notices_;
};
}

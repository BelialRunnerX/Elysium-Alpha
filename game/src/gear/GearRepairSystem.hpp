// Intended function: Track item damage, repair recipes, material cost, repair skill, and quality loss.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct GearRepairSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct GearRepairSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct GearRepairSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class GearRepairSystemSystem {
public:
 bool submit(const GearRepairSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const GearRepairSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<GearRepairSystemRecord> snapshot() const;
 std::vector<GearRepairSystemNotice> drainNotices(); void clear();
private:
 GearRepairSystemRecord* mutableFind(std::uint64_t); void notice(const GearRepairSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<GearRepairSystemRecord> records_; std::vector<GearRepairSystemNotice> notices_;
};
}

// Intended function: Track upgrade tiers, required components, success rules, costs, and stable item identity.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct GearUpgradeSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct GearUpgradeSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct GearUpgradeSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class GearUpgradeSystemSystem {
public:
 bool submit(const GearUpgradeSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const GearUpgradeSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<GearUpgradeSystemRecord> snapshot() const;
 std::vector<GearUpgradeSystemNotice> drainNotices(); void clear();
private:
 GearUpgradeSystemRecord* mutableFind(std::uint64_t); void notice(const GearUpgradeSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<GearUpgradeSystemRecord> records_; std::vector<GearUpgradeSystemNotice> notices_;
};
}

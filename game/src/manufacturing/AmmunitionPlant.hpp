// Intended function: Track propellant/energy cells, casings, warheads, quality, safety, and ammunition output.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct AmmunitionPlantRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AmmunitionPlantRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AmmunitionPlantNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AmmunitionPlantSystem {
public:
 bool submit(const AmmunitionPlantRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AmmunitionPlantRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AmmunitionPlantRecord> snapshot() const;
 std::vector<AmmunitionPlantNotice> drainNotices(); void clear();
private:
 AmmunitionPlantRecord* mutableFind(std::uint64_t); void notice(const AmmunitionPlantRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AmmunitionPlantRecord> records_; std::vector<AmmunitionPlantNotice> notices_;
};
}

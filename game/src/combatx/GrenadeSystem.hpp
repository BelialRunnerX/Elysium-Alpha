// Intended function: Track thrown explosive trajectories, fuse timing, area effects, cover attenuation, and hazards.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::combatx {
struct GrenadeSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct GrenadeSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct GrenadeSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class GrenadeSystemSystem {
public:
 bool submit(const GrenadeSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const GrenadeSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<GrenadeSystemRecord> snapshot() const;
 std::vector<GrenadeSystemNotice> drainNotices(); void clear();
private:
 GrenadeSystemRecord* mutableFind(std::uint64_t); void notice(const GrenadeSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<GrenadeSystemRecord> records_; std::vector<GrenadeSystemNotice> notices_;
};
}

// Intended function: Track weapon sockets, compatible mods, stat transforms, durability, and provenance.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct WeaponModSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct WeaponModSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct WeaponModSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class WeaponModSystemSystem {
public:
 bool submit(const WeaponModSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const WeaponModSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<WeaponModSystemRecord> snapshot() const;
 std::vector<WeaponModSystemNotice> drainNotices(); void clear();
private:
 WeaponModSystemRecord* mutableFind(std::uint64_t); void notice(const WeaponModSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<WeaponModSystemRecord> records_; std::vector<WeaponModSystemNotice> notices_;
};
}

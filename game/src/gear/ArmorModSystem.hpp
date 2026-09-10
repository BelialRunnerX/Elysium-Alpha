// Intended function: Track armor sockets, defensive mods, resistances, utility effects, and integrity.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct ArmorModSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ArmorModSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ArmorModSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ArmorModSystemSystem {
public:
 bool submit(const ArmorModSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ArmorModSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ArmorModSystemRecord> snapshot() const;
 std::vector<ArmorModSystemNotice> drainNotices(); void clear();
private:
 ArmorModSystemRecord* mutableFind(std::uint64_t); void notice(const ArmorModSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ArmorModSystemRecord> records_; std::vector<ArmorModSystemNotice> notices_;
};
}

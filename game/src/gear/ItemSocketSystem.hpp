// Intended function: Track rune/chip/gem sockets, compatibility, insertion/removal, and resulting modifiers.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct ItemSocketSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ItemSocketSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ItemSocketSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ItemSocketSystemSystem {
public:
 bool submit(const ItemSocketSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ItemSocketSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ItemSocketSystemRecord> snapshot() const;
 std::vector<ItemSocketSystemNotice> drainNotices(); void clear();
private:
 ItemSocketSystemRecord* mutableFind(std::uint64_t); void notice(const ItemSocketSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ItemSocketSystemRecord> records_; std::vector<ItemSocketSystemNotice> notices_;
};
}

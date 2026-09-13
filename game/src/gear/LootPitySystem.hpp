// Intended function: Track deterministic bad-luck protection without bypassing authoritative loot tables.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct LootPitySystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct LootPitySystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct LootPitySystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class LootPitySystemSystem {
public:
 bool submit(const LootPitySystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const LootPitySystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<LootPitySystemRecord> snapshot() const;
 std::vector<LootPitySystemNotice> drainNotices(); void clear();
private:
 LootPitySystemRecord* mutableFind(std::uint64_t); void notice(const LootPitySystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<LootPitySystemRecord> records_; std::vector<LootPitySystemNotice> notices_;
};
}

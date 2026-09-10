// Intended function: Track item rarity/tier bands from threat, region, content progression, and source class.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct LootTierSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct LootTierSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct LootTierSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class LootTierSystemSystem {
public:
 bool submit(const LootTierSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const LootTierSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<LootTierSystemRecord> snapshot() const;
 std::vector<LootTierSystemNotice> drainNotices(); void clear();
private:
 LootTierSystemRecord* mutableFind(std::uint64_t); void notice(const LootTierSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<LootTierSystemRecord> records_; std::vector<LootTierSystemNotice> notices_;
};
}

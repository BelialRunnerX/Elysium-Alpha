// Intended function: Track unidentified properties, analysis progress, reveal thresholds, and discovery source.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct ItemIdentificationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ItemIdentificationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ItemIdentificationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ItemIdentificationSystem {
public:
 bool submit(const ItemIdentificationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ItemIdentificationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ItemIdentificationRecord> snapshot() const;
 std::vector<ItemIdentificationNotice> drainNotices(); void clear();
private:
 ItemIdentificationRecord* mutableFind(std::uint64_t); void notice(const ItemIdentificationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ItemIdentificationRecord> records_; std::vector<ItemIdentificationNotice> notices_;
};
}

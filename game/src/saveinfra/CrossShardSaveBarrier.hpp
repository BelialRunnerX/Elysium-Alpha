// Intended function: Coordinate durable publication when state spans multiple active/sleeping shards.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct CrossShardSaveBarrierRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CrossShardSaveBarrierRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CrossShardSaveBarrierNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CrossShardSaveBarrierSystem {
public:
 bool submit(const CrossShardSaveBarrierRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CrossShardSaveBarrierRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CrossShardSaveBarrierRecord> snapshot() const;
 std::vector<CrossShardSaveBarrierNotice> drainNotices(); void clear();
private:
 CrossShardSaveBarrierRecord* mutableFind(std::uint64_t); void notice(const CrossShardSaveBarrierRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CrossShardSaveBarrierRecord> records_; std::vector<CrossShardSaveBarrierNotice> notices_;
};
}

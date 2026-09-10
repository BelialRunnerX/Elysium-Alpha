// Intended function: Merge worker outputs in canonical key order before owner-system publication.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct ParallelMergeBarrierRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ParallelMergeBarrierRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ParallelMergeBarrierNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ParallelMergeBarrierSystem {
public:
 bool submit(const ParallelMergeBarrierRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ParallelMergeBarrierRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ParallelMergeBarrierRecord> snapshot() const;
 std::vector<ParallelMergeBarrierNotice> drainNotices(); void clear();
private:
 ParallelMergeBarrierRecord* mutableFind(std::uint64_t); void notice(const ParallelMergeBarrierRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ParallelMergeBarrierRecord> records_; std::vector<ParallelMergeBarrierNotice> notices_;
};
}

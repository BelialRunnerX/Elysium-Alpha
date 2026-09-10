// Intended function: Schedule only dirty/active bounded field regions for atmosphere, heat, fluid, ecology, and hazards.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct SparseFieldSchedulerRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SparseFieldSchedulerRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SparseFieldSchedulerNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SparseFieldSchedulerSystem {
public:
 bool submit(const SparseFieldSchedulerRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SparseFieldSchedulerRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SparseFieldSchedulerRecord> snapshot() const;
 std::vector<SparseFieldSchedulerNotice> drainNotices(); void clear();
private:
 SparseFieldSchedulerRecord* mutableFind(std::uint64_t); void notice(const SparseFieldSchedulerRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SparseFieldSchedulerRecord> records_; std::vector<SparseFieldSchedulerNotice> notices_;
};
}

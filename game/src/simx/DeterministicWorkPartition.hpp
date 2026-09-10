// Intended function: Partition address/stable-ID work into deterministic worker batches for future multithreading.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct DeterministicWorkPartitionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct DeterministicWorkPartitionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct DeterministicWorkPartitionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class DeterministicWorkPartitionSystem {
public:
 bool submit(const DeterministicWorkPartitionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const DeterministicWorkPartitionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<DeterministicWorkPartitionRecord> snapshot() const;
 std::vector<DeterministicWorkPartitionNotice> drainNotices(); void clear();
private:
 DeterministicWorkPartitionRecord* mutableFind(std::uint64_t); void notice(const DeterministicWorkPartitionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<DeterministicWorkPartitionRecord> records_; std::vector<DeterministicWorkPartitionNotice> notices_;
};
}

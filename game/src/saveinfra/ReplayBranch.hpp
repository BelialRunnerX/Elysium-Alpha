// Intended function: Track alternate replay branches, parent checkpoints, divergent commands, and metadata.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct ReplayBranchRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ReplayBranchRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ReplayBranchNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ReplayBranchSystem {
public:
 bool submit(const ReplayBranchRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ReplayBranchRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ReplayBranchRecord> snapshot() const;
 std::vector<ReplayBranchNotice> drainNotices(); void clear();
private:
 ReplayBranchRecord* mutableFind(std::uint64_t); void notice(const ReplayBranchRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ReplayBranchRecord> records_; std::vector<ReplayBranchNotice> notices_;
};
}

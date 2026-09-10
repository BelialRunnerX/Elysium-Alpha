// Intended function: Track periodic deterministic replay checkpoints and input-range coverage.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct ReplayCheckpointRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ReplayCheckpointRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ReplayCheckpointNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ReplayCheckpointSystem {
public:
 bool submit(const ReplayCheckpointRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ReplayCheckpointRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ReplayCheckpointRecord> snapshot() const;
 std::vector<ReplayCheckpointNotice> drainNotices(); void clear();
private:
 ReplayCheckpointRecord* mutableFind(std::uint64_t); void notice(const ReplayCheckpointRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ReplayCheckpointRecord> records_; std::vector<ReplayCheckpointNotice> notices_;
};
}

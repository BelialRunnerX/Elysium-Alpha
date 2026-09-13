// Intended function: Reserve AI tasks by stable actor/job identity with leases and deterministic conflict resolution.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct TaskClaimSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct TaskClaimSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct TaskClaimSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class TaskClaimSystemSystem {
public:
 bool submit(const TaskClaimSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const TaskClaimSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<TaskClaimSystemRecord> snapshot() const;
 std::vector<TaskClaimSystemNotice> drainNotices(); void clear();
private:
 TaskClaimSystemRecord* mutableFind(std::uint64_t); void notice(const TaskClaimSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<TaskClaimSystemRecord> records_; std::vector<TaskClaimSystemNotice> notices_;
};
}

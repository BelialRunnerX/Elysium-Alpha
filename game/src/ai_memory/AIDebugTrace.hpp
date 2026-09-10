// Intended function: Record compact planner decision traces for later debugging and Why Inspector integration.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct AIDebugTraceRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AIDebugTraceRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AIDebugTraceNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AIDebugTraceSystem {
public:
 bool submit(const AIDebugTraceRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AIDebugTraceRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AIDebugTraceRecord> snapshot() const;
 std::vector<AIDebugTraceNotice> drainNotices(); void clear();
private:
 AIDebugTraceRecord* mutableFind(std::uint64_t); void notice(const AIDebugTraceRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AIDebugTraceRecord> records_; std::vector<AIDebugTraceNotice> notices_;
};
}

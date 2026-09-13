// Intended function: Project goals, plans, memory, reservations, failures, and decision utility breakdowns.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct AIInspectorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AIInspectorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AIInspectorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AIInspectorSystem {
public:
 bool submit(const AIInspectorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AIInspectorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AIInspectorRecord> snapshot() const;
 std::vector<AIInspectorNotice> drainNotices(); void clear();
private:
 AIInspectorRecord* mutableFind(std::uint64_t); void notice(const AIInspectorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AIInspectorRecord> records_; std::vector<AIInspectorNotice> notices_;
};
}

// Intended function: Track urgent planner interrupts such as hazards, attacks, medical needs, and critical shortages.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct AIInterruptsRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AIInterruptsRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AIInterruptsNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AIInterruptsSystem {
public:
 bool submit(const AIInterruptsRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AIInterruptsRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AIInterruptsRecord> snapshot() const;
 std::vector<AIInterruptsNotice> drainNotices(); void clear();
private:
 AIInterruptsRecord* mutableFind(std::uint64_t); void notice(const AIInterruptsRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AIInterruptsRecord> records_; std::vector<AIInterruptsNotice> notices_;
};
}

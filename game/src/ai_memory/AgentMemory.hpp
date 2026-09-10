// Intended function: Track bounded actor memories with salience, confidence, decay, source, and event references.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct AgentMemoryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AgentMemoryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AgentMemoryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AgentMemorySystem {
public:
 bool submit(const AgentMemoryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AgentMemoryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AgentMemoryRecord> snapshot() const;
 std::vector<AgentMemoryNotice> drainNotices(); void clear();
private:
 AgentMemoryRecord* mutableFind(std::uint64_t); void notice(const AgentMemoryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AgentMemoryRecord> records_; std::vector<AgentMemoryNotice> notices_;
};
}

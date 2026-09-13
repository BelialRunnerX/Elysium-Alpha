// Intended function: Track dangerous actors/sites/events with recency, severity, confidence, and avoidance behavior.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct ThreatMemoryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ThreatMemoryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ThreatMemoryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ThreatMemorySystem {
public:
 bool submit(const ThreatMemoryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ThreatMemoryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ThreatMemoryRecord> snapshot() const;
 std::vector<ThreatMemoryNotice> drainNotices(); void clear();
private:
 ThreatMemoryRecord* mutableFind(std::uint64_t); void notice(const ThreatMemoryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ThreatMemoryRecord> records_; std::vector<ThreatMemoryNotice> notices_;
};
}

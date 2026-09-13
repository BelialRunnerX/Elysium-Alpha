// Intended function: Track per-system timing counters, work units, backlog, and budget overruns.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct SystemProfilerRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SystemProfilerRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SystemProfilerNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SystemProfilerSystem {
public:
 bool submit(const SystemProfilerRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SystemProfilerRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SystemProfilerRecord> snapshot() const;
 std::vector<SystemProfilerNotice> drainNotices(); void clear();
private:
 SystemProfilerRecord* mutableFind(std::uint64_t); void notice(const SystemProfilerRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SystemProfilerRecord> records_; std::vector<SystemProfilerNotice> notices_;
};
}

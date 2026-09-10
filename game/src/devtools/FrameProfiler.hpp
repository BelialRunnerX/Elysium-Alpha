// Intended function: Track renderer-neutral frame phases, update/render latency, stalls, and percentile summaries.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct FrameProfilerRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct FrameProfilerRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct FrameProfilerNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class FrameProfilerSystem {
public:
 bool submit(const FrameProfilerRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const FrameProfilerRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<FrameProfilerRecord> snapshot() const;
 std::vector<FrameProfilerNotice> drainNotices(); void clear();
private:
 FrameProfilerRecord* mutableFind(std::uint64_t); void notice(const FrameProfilerRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<FrameProfilerRecord> records_; std::vector<FrameProfilerNotice> notices_;
};
}

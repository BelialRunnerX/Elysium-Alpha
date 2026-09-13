// Intended function: Track endangered actors, hazards, extraction conditions, timers, and safe recovery.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct RescueMissionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct RescueMissionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct RescueMissionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class RescueMissionSystem {
public:
 bool submit(const RescueMissionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const RescueMissionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<RescueMissionRecord> snapshot() const;
 std::vector<RescueMissionNotice> drainNotices(); void clear();
private:
 RescueMissionRecord* mutableFind(std::uint64_t); void notice(const RescueMissionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<RescueMissionRecord> records_; std::vector<RescueMissionNotice> notices_;
};
}

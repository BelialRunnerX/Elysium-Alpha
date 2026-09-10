// Intended function: Track escorted actors/convoys, route segments, threat, separation, and arrival.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct EscortMissionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct EscortMissionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct EscortMissionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class EscortMissionSystem {
public:
 bool submit(const EscortMissionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const EscortMissionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<EscortMissionRecord> snapshot() const;
 std::vector<EscortMissionNotice> drainNotices(); void clear();
private:
 EscortMissionRecord* mutableFind(std::uint64_t); void notice(const EscortMissionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<EscortMissionRecord> records_; std::vector<EscortMissionNotice> notices_;
};
}

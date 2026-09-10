// Intended function: Track negotiation objectives, required leverage, meeting state, incidents, and outcomes.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct DiplomaticMissionSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct DiplomaticMissionSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct DiplomaticMissionSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class DiplomaticMissionSystemSystem {
public:
 bool submit(const DiplomaticMissionSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const DiplomaticMissionSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<DiplomaticMissionSystemRecord> snapshot() const;
 std::vector<DiplomaticMissionSystemNotice> drainNotices(); void clear();
private:
 DiplomaticMissionSystemRecord* mutableFind(std::uint64_t); void notice(const DiplomaticMissionSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<DiplomaticMissionSystemRecord> records_; std::vector<DiplomaticMissionSystemNotice> notices_;
};
}

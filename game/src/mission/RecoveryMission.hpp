// Intended function: Track recoverable objects, claims, hazards, extraction, and delivery requirements.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct RecoveryMissionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct RecoveryMissionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct RecoveryMissionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class RecoveryMissionSystem {
public:
 bool submit(const RecoveryMissionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const RecoveryMissionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<RecoveryMissionRecord> snapshot() const;
 std::vector<RecoveryMissionNotice> drainNotices(); void clear();
private:
 RecoveryMissionRecord* mutableFind(std::uint64_t); void notice(const RecoveryMissionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<RecoveryMissionRecord> records_; std::vector<RecoveryMissionNotice> notices_;
};
}

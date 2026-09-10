// Intended function: Track defended sites, waves, strategic conditions, damage limits, and success/failure.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct DefenseMissionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct DefenseMissionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct DefenseMissionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class DefenseMissionSystem {
public:
 bool submit(const DefenseMissionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const DefenseMissionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<DefenseMissionRecord> snapshot() const;
 std::vector<DefenseMissionNotice> drainNotices(); void clear();
private:
 DefenseMissionRecord* mutableFind(std::uint64_t); void notice(const DefenseMissionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<DefenseMissionRecord> records_; std::vector<DefenseMissionNotice> notices_;
};
}

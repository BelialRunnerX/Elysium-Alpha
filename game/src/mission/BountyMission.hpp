// Intended function: Track bounty target identity, evidence, location uncertainty, capture/kill terms, and rewards.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct BountyMissionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct BountyMissionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct BountyMissionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class BountyMissionSystem {
public:
 bool submit(const BountyMissionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const BountyMissionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<BountyMissionRecord> snapshot() const;
 std::vector<BountyMissionNotice> drainNotices(); void clear();
private:
 BountyMissionRecord* mutableFind(std::uint64_t); void notice(const BountyMissionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<BountyMissionRecord> records_; std::vector<BountyMissionNotice> notices_;
};
}

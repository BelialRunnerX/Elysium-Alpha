// Intended function: Track build-site objectives, required materials, milestones, deadlines, and commissioning.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct ConstructionMissionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ConstructionMissionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ConstructionMissionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ConstructionMissionSystem {
public:
 bool submit(const ConstructionMissionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ConstructionMissionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ConstructionMissionRecord> snapshot() const;
 std::vector<ConstructionMissionNotice> drainNotices(); void clear();
private:
 ConstructionMissionRecord* mutableFind(std::uint64_t); void notice(const ConstructionMissionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ConstructionMissionRecord> records_; std::vector<ConstructionMissionNotice> notices_;
};
}

// Intended function: Track anchor, tether, counterweight, climber, power, and safety milestones for space elevators.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::megaproject {
struct SpaceElevatorProjectRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SpaceElevatorProjectRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SpaceElevatorProjectNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SpaceElevatorProjectSystem {
public:
 bool submit(const SpaceElevatorProjectRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SpaceElevatorProjectRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SpaceElevatorProjectRecord> snapshot() const;
 std::vector<SpaceElevatorProjectNotice> drainNotices(); void clear();
private:
 SpaceElevatorProjectRecord* mutableFind(std::uint64_t); void notice(const SpaceElevatorProjectRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SpaceElevatorProjectRecord> records_; std::vector<SpaceElevatorProjectNotice> notices_;
};
}

// Intended function: Select and advance systemic missions from world state, faction needs, danger, and player history.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct MissionDirectorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct MissionDirectorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct MissionDirectorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class MissionDirectorSystem {
public:
 bool submit(const MissionDirectorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const MissionDirectorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<MissionDirectorRecord> snapshot() const;
 std::vector<MissionDirectorNotice> drainNotices(); void clear();
private:
 MissionDirectorRecord* mutableFind(std::uint64_t); void notice(const MissionDirectorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<MissionDirectorRecord> records_; std::vector<MissionDirectorNotice> notices_;
};
}

// Intended function: Project active/completed/failed missions, objective graphs, rewards, and world references.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct MissionJournalViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct MissionJournalViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct MissionJournalViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class MissionJournalViewSystem {
public:
 bool submit(const MissionJournalViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const MissionJournalViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<MissionJournalViewRecord> snapshot() const;
 std::vector<MissionJournalViewNotice> drainNotices(); void clear();
private:
 MissionJournalViewRecord* mutableFind(std::uint64_t); void notice(const MissionJournalViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<MissionJournalViewRecord> records_; std::vector<MissionJournalViewNotice> notices_;
};
}

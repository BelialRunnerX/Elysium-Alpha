// Intended function: Track samples, scans, experiments, anomalies, and research-delivery objectives.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct ResearchMissionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ResearchMissionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ResearchMissionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ResearchMissionSystem {
public:
 bool submit(const ResearchMissionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ResearchMissionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ResearchMissionRecord> snapshot() const;
 std::vector<ResearchMissionNotice> drainNotices(); void clear();
private:
 ResearchMissionRecord* mutableFind(std::uint64_t); void notice(const ResearchMissionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ResearchMissionRecord> records_; std::vector<ResearchMissionNotice> notices_;
};
}

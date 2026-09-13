// Intended function: Track system/planet/site survey requirements, confidence, discoveries, and submission.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct SurveyMissionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SurveyMissionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SurveyMissionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SurveyMissionSystem {
public:
 bool submit(const SurveyMissionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SurveyMissionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SurveyMissionRecord> snapshot() const;
 std::vector<SurveyMissionNotice> drainNotices(); void clear();
private:
 SurveyMissionRecord* mutableFind(std::uint64_t); void notice(const SurveyMissionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SurveyMissionRecord> records_; std::vector<SurveyMissionNotice> notices_;
};
}

// Intended function: Track mapped geology, biosphere, hazards, resources, ruins, and confidence per survey region.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct PlanetarySurveyGridRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct PlanetarySurveyGridRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct PlanetarySurveyGridNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class PlanetarySurveyGridSystem {
public:
 bool submit(const PlanetarySurveyGridRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const PlanetarySurveyGridRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<PlanetarySurveyGridRecord> snapshot() const;
 std::vector<PlanetarySurveyGridNotice> drainNotices(); void clear();
private:
 PlanetarySurveyGridRecord* mutableFind(std::uint64_t); void notice(const PlanetarySurveyGridRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<PlanetarySurveyGridRecord> records_; std::vector<PlanetarySurveyGridNotice> notices_;
};
}

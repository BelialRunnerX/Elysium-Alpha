// Intended function: Project civilization population, economy, doctrine, cohesion, technology, wars, and history.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct CivilizationViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CivilizationViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CivilizationViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CivilizationViewSystem {
public:
 bool submit(const CivilizationViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CivilizationViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CivilizationViewRecord> snapshot() const;
 std::vector<CivilizationViewNotice> drainNotices(); void clear();
private:
 CivilizationViewRecord* mutableFind(std::uint64_t); void notice(const CivilizationViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CivilizationViewRecord> records_; std::vector<CivilizationViewNotice> notices_;
};
}

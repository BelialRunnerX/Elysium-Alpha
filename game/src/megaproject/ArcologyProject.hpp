// Intended function: Track arcology shells, utilities, habitation sectors, service capacity, and population onboarding.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::megaproject {
struct ArcologyProjectRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ArcologyProjectRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ArcologyProjectNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ArcologyProjectSystem {
public:
 bool submit(const ArcologyProjectRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ArcologyProjectRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ArcologyProjectRecord> snapshot() const;
 std::vector<ArcologyProjectNotice> drainNotices(); void clear();
private:
 ArcologyProjectRecord* mutableFind(std::uint64_t); void notice(const ArcologyProjectRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ArcologyProjectRecord> records_; std::vector<ArcologyProjectNotice> notices_;
};
}

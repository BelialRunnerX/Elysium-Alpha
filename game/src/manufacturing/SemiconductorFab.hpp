// Intended function: Track wafer lots, process stages, contamination, yield, and electronics-grade output.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct SemiconductorFabRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SemiconductorFabRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SemiconductorFabNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SemiconductorFabSystem {
public:
 bool submit(const SemiconductorFabRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SemiconductorFabRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SemiconductorFabRecord> snapshot() const;
 std::vector<SemiconductorFabNotice> drainNotices(); void clear();
private:
 SemiconductorFabRecord* mutableFind(std::uint64_t); void notice(const SemiconductorFabRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SemiconductorFabRecord> records_; std::vector<SemiconductorFabNotice> notices_;
};
}

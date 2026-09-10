// Intended function: Track generator sites, coverage, synchronization, power demand, and shield readiness.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::megaproject {
struct PlanetaryShieldProjectRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct PlanetaryShieldProjectRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct PlanetaryShieldProjectNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class PlanetaryShieldProjectSystem {
public:
 bool submit(const PlanetaryShieldProjectRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const PlanetaryShieldProjectRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<PlanetaryShieldProjectRecord> snapshot() const;
 std::vector<PlanetaryShieldProjectNotice> drainNotices(); void clear();
private:
 PlanetaryShieldProjectRecord* mutableFind(std::uint64_t); void notice(const PlanetaryShieldProjectRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<PlanetaryShieldProjectRecord> records_; std::vector<PlanetaryShieldProjectNotice> notices_;
};
}

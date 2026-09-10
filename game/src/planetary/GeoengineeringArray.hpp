// Intended function: Track albedo, aerosol, cloud-seeding, thermal, and precipitation interventions.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct GeoengineeringArrayRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct GeoengineeringArrayRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct GeoengineeringArrayNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class GeoengineeringArraySystem {
public:
 bool submit(const GeoengineeringArrayRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const GeoengineeringArrayRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<GeoengineeringArrayRecord> snapshot() const;
 std::vector<GeoengineeringArrayNotice> drainNotices(); void clear();
private:
 GeoengineeringArrayRecord* mutableFind(std::uint64_t); void notice(const GeoengineeringArrayRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<GeoengineeringArrayRecord> records_; std::vector<GeoengineeringArrayNotice> notices_;
};
}

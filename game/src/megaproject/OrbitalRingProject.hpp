// Intended function: Track staged orbital-ring construction, material demand, orbital sectors, workforce, and commissioning.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::megaproject {
struct OrbitalRingProjectRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct OrbitalRingProjectRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct OrbitalRingProjectNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class OrbitalRingProjectSystem {
public:
 bool submit(const OrbitalRingProjectRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const OrbitalRingProjectRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<OrbitalRingProjectRecord> snapshot() const;
 std::vector<OrbitalRingProjectNotice> drainNotices(); void clear();
private:
 OrbitalRingProjectRecord* mutableFind(std::uint64_t); void notice(const OrbitalRingProjectRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<OrbitalRingProjectRecord> records_; std::vector<OrbitalRingProjectNotice> notices_;
};
}

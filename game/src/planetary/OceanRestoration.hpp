// Intended function: Track acidity, oxygen, nutrients, contamination, ecology, and restoration interventions.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct OceanRestorationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct OceanRestorationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct OceanRestorationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class OceanRestorationSystem {
public:
 bool submit(const OceanRestorationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const OceanRestorationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<OceanRestorationRecord> snapshot() const;
 std::vector<OceanRestorationNotice> drainNotices(); void clear();
private:
 OceanRestorationRecord* mutableFind(std::uint64_t); void notice(const OceanRestorationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<OceanRestorationRecord> records_; std::vector<OceanRestorationNotice> notices_;
};
}

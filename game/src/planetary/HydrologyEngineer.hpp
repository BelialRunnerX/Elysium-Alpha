// Intended function: Track aquifers, rivers, reservoirs, drainage, irrigation, flooding, and water-table interventions.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct HydrologyEngineerRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct HydrologyEngineerRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct HydrologyEngineerNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class HydrologyEngineerSystem {
public:
 bool submit(const HydrologyEngineerRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const HydrologyEngineerRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<HydrologyEngineerRecord> snapshot() const;
 std::vector<HydrologyEngineerNotice> drainNotices(); void clear();
private:
 HydrologyEngineerRecord* mutableFind(std::uint64_t); void notice(const HydrologyEngineerRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<HydrologyEngineerRecord> records_; std::vector<HydrologyEngineerNotice> notices_;
};
}

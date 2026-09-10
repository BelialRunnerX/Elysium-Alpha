// Intended function: Project atmosphere, climate, hydrology, soil, biosphere, milestones, and interventions.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct TerraformingViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct TerraformingViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct TerraformingViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class TerraformingViewSystem {
public:
 bool submit(const TerraformingViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const TerraformingViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<TerraformingViewRecord> snapshot() const;
 std::vector<TerraformingViewNotice> drainNotices(); void clear();
private:
 TerraformingViewRecord* mutableFind(std::uint64_t); void notice(const TerraformingViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<TerraformingViewRecord> records_; std::vector<TerraformingViewNotice> notices_;
};
}

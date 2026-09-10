// Intended function: Track atmosphere, hydrology, soil, biosphere, and climate milestones around terraforming spires.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::megaproject {
struct TerraformSpireProjectRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct TerraformSpireProjectRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct TerraformSpireProjectNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class TerraformSpireProjectSystem {
public:
 bool submit(const TerraformSpireProjectRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const TerraformSpireProjectRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<TerraformSpireProjectRecord> snapshot() const;
 std::vector<TerraformSpireProjectNotice> drainNotices(); void clear();
private:
 TerraformSpireProjectRecord* mutableFind(std::uint64_t); void notice(const TerraformSpireProjectRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<TerraformSpireProjectRecord> records_; std::vector<TerraformSpireProjectNotice> notices_;
};
}

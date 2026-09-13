// Intended function: Track zero-g foundry modules, feedstock, power, fabrication capacity, and strategic output.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::megaproject {
struct InterstellarFoundryProjectRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct InterstellarFoundryProjectRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct InterstellarFoundryProjectNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class InterstellarFoundryProjectSystem {
public:
 bool submit(const InterstellarFoundryProjectRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const InterstellarFoundryProjectRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<InterstellarFoundryProjectRecord> snapshot() const;
 std::vector<InterstellarFoundryProjectNotice> drainNotices(); void clear();
private:
 InterstellarFoundryProjectRecord* mutableFind(std::uint64_t); void notice(const InterstellarFoundryProjectRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<InterstellarFoundryProjectRecord> records_; std::vector<InterstellarFoundryProjectNotice> notices_;
};
}

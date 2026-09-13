// Intended function: Track deterministic item dismantling into reusable materials while preserving provenance hooks.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct SalvageBreakdownRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SalvageBreakdownRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SalvageBreakdownNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SalvageBreakdownSystem {
public:
 bool submit(const SalvageBreakdownRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SalvageBreakdownRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SalvageBreakdownRecord> snapshot() const;
 std::vector<SalvageBreakdownNotice> drainNotices(); void clear();
private:
 SalvageBreakdownRecord* mutableFind(std::uint64_t); void notice(const SalvageBreakdownRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SalvageBreakdownRecord> records_; std::vector<SalvageBreakdownNotice> notices_;
};
}

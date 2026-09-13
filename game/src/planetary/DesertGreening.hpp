// Intended function: Track water, soil cover, vegetation, windbreaks, and desertification reversal.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct DesertGreeningRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct DesertGreeningRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct DesertGreeningNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class DesertGreeningSystem {
public:
 bool submit(const DesertGreeningRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const DesertGreeningRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<DesertGreeningRecord> snapshot() const;
 std::vector<DesertGreeningNotice> drainNotices(); void clear();
private:
 DesertGreeningRecord* mutableFind(std::uint64_t); void notice(const DesertGreeningRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<DesertGreeningRecord> records_; std::vector<DesertGreeningNotice> notices_;
};
}

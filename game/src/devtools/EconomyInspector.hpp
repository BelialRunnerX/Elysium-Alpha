// Intended function: Project market, trade, stock, route, price, and settlement indicators.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct EconomyInspectorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct EconomyInspectorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct EconomyInspectorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class EconomyInspectorSystem {
public:
 bool submit(const EconomyInspectorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const EconomyInspectorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<EconomyInspectorRecord> snapshot() const;
 std::vector<EconomyInspectorNotice> drainNotices(); void clear();
private:
 EconomyInspectorRecord* mutableFind(std::uint64_t); void notice(const EconomyInspectorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<EconomyInspectorRecord> records_; std::vector<EconomyInspectorNotice> notices_;
};
}

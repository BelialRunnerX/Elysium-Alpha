// Intended function: Project ship construction/refit stages, modules, resources, labor, tests, and launch readiness.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct ShipyardViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipyardViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipyardViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipyardViewSystem {
public:
 bool submit(const ShipyardViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipyardViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipyardViewRecord> snapshot() const;
 std::vector<ShipyardViewNotice> drainNotices(); void clear();
private:
 ShipyardViewRecord* mutableFind(std::uint64_t); void notice(const ShipyardViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipyardViewRecord> records_; std::vector<ShipyardViewNotice> notices_;
};
}

// Intended function: Track ship sections, systems integration, fitting, pressure tests, and launch readiness.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct ShipyardAssemblyRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipyardAssemblyRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipyardAssemblyNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipyardAssemblySystem {
public:
 bool submit(const ShipyardAssemblyRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipyardAssemblyRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipyardAssemblyRecord> snapshot() const;
 std::vector<ShipyardAssemblyNotice> drainNotices(); void clear();
private:
 ShipyardAssemblyRecord* mutableFind(std::uint64_t); void notice(const ShipyardAssemblyRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipyardAssemblyRecord> records_; std::vector<ShipyardAssemblyNotice> notices_;
};
}

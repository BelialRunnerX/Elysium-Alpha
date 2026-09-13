// Intended function: Track defensive arcs, ammunition/energy, target assignments, saturation, and readiness.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipPointDefenseRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipPointDefenseRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipPointDefenseNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipPointDefenseSystem {
public:
 bool submit(const ShipPointDefenseRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipPointDefenseRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipPointDefenseRecord> snapshot() const;
 std::vector<ShipPointDefenseNotice> drainNotices(); void clear();
private:
 ShipPointDefenseRecord* mutableFind(std::uint64_t); void notice(const ShipPointDefenseRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipPointDefenseRecord> records_; std::vector<ShipPointDefenseNotice> notices_;
};
}

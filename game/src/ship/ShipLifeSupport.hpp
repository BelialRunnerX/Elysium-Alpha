// Intended function: Track atmosphere, water, food, waste, reserve margins, and compartment support state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipLifeSupportRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipLifeSupportRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipLifeSupportNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipLifeSupportSystem {
public:
 bool submit(const ShipLifeSupportRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipLifeSupportRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipLifeSupportRecord> snapshot() const;
 std::vector<ShipLifeSupportNotice> drainNotices(); void clear();
private:
 ShipLifeSupportRecord* mutableFind(std::uint64_t); void notice(const ShipLifeSupportRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipLifeSupportRecord> records_; std::vector<ShipLifeSupportNotice> notices_;
};
}

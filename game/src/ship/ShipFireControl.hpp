// Intended function: Track shipboard fires, suppression zones, atmosphere isolation, damage, and extinguishing progress.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipFireControlRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipFireControlRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipFireControlNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipFireControlSystem {
public:
 bool submit(const ShipFireControlRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipFireControlRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipFireControlRecord> snapshot() const;
 std::vector<ShipFireControlNotice> drainNotices(); void clear();
private:
 ShipFireControlRecord* mutableFind(std::uint64_t); void notice(const ShipFireControlRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipFireControlRecord> records_; std::vector<ShipFireControlNotice> notices_;
};
}

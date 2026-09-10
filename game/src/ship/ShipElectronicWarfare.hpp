// Intended function: Track jamming, deception, countermeasures, signature, and sensor disruption.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipElectronicWarfareRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipElectronicWarfareRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipElectronicWarfareNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipElectronicWarfareSystem {
public:
 bool submit(const ShipElectronicWarfareRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipElectronicWarfareRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipElectronicWarfareRecord> snapshot() const;
 std::vector<ShipElectronicWarfareNotice> drainNotices(); void clear();
private:
 ShipElectronicWarfareRecord* mutableFind(std::uint64_t); void notice(const ShipElectronicWarfareRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipElectronicWarfareRecord> records_; std::vector<ShipElectronicWarfareNotice> notices_;
};
}

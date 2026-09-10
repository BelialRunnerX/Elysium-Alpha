// Intended function: Track derelict survey, hazards, claims, salvage targets, extraction, and tow decisions.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipSalvageOperationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipSalvageOperationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipSalvageOperationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipSalvageOperationSystem {
public:
 bool submit(const ShipSalvageOperationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipSalvageOperationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipSalvageOperationRecord> snapshot() const;
 std::vector<ShipSalvageOperationNotice> drainNotices(); void clear();
private:
 ShipSalvageOperationRecord* mutableFind(std::uint64_t); void notice(const ShipSalvageOperationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipSalvageOperationRecord> records_; std::vector<ShipSalvageOperationNotice> notices_;
};
}

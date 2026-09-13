// Intended function: Schedule field repairs by severity, parts, crew, tools, access, and operational priority.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipRepairQueueRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipRepairQueueRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipRepairQueueNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipRepairQueueSystem {
public:
 bool submit(const ShipRepairQueueRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipRepairQueueRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipRepairQueueRecord> snapshot() const;
 std::vector<ShipRepairQueueNotice> drainNotices(); void clear();
private:
 ShipRepairQueueRecord* mutableFind(std::uint64_t); void notice(const ShipRepairQueueRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipRepairQueueRecord> records_; std::vector<ShipRepairQueueNotice> notices_;
};
}

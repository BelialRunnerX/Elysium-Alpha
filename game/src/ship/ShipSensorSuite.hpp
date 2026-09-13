// Intended function: Track sensor modes, contacts, emissions, confidence, resolution, and tracking state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipSensorSuiteRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipSensorSuiteRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipSensorSuiteNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipSensorSuiteSystem {
public:
 bool submit(const ShipSensorSuiteRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipSensorSuiteRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipSensorSuiteRecord> snapshot() const;
 std::vector<ShipSensorSuiteNotice> drainNotices(); void clear();
private:
 ShipSensorSuiteRecord* mutableFind(std::uint64_t); void notice(const ShipSensorSuiteRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipSensorSuiteRecord> records_; std::vector<ShipSensorSuiteNotice> notices_;
};
}

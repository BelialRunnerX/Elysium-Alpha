// Intended function: Track cell chemistry, materials, formation cycles, quality, safety, and pack assembly.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct BatteryFactoryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct BatteryFactoryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct BatteryFactoryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class BatteryFactorySystem {
public:
 bool submit(const BatteryFactoryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const BatteryFactoryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<BatteryFactoryRecord> snapshot() const;
 std::vector<BatteryFactoryNotice> drainNotices(); void clear();
private:
 BatteryFactoryRecord* mutableFind(std::uint64_t); void notice(const BatteryFactoryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<BatteryFactoryRecord> records_; std::vector<BatteryFactoryNotice> notices_;
};
}

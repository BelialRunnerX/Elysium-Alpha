// Intended function: Track chassis, actuators, sensors, control units, testing, and drone commissioning.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct DroneAssemblyRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct DroneAssemblyRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct DroneAssemblyNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class DroneAssemblySystem {
public:
 bool submit(const DroneAssemblyRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const DroneAssemblyRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<DroneAssemblyRecord> snapshot() const;
 std::vector<DroneAssemblyNotice> drainNotices(); void clear();
private:
 DroneAssemblyRecord* mutableFind(std::uint64_t); void notice(const DroneAssemblyRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<DroneAssemblyRecord> records_; std::vector<DroneAssemblyNotice> notices_;
};
}

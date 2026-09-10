// Intended function: Track precipitation, storm suppression, hail control, cloud seeding, and side effects.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct WeatherModificationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct WeatherModificationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct WeatherModificationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class WeatherModificationSystem {
public:
 bool submit(const WeatherModificationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const WeatherModificationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<WeatherModificationRecord> snapshot() const;
 std::vector<WeatherModificationNotice> drainNotices(); void clear();
private:
 WeatherModificationRecord* mutableFind(std::uint64_t); void notice(const WeatherModificationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<WeatherModificationRecord> records_; std::vector<WeatherModificationNotice> notices_;
};
}

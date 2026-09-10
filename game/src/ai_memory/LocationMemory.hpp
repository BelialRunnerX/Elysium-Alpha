// Intended function: Track known routes, shelters, hazards, work sites, and points of interest.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct LocationMemoryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct LocationMemoryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct LocationMemoryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class LocationMemorySystem {
public:
 bool submit(const LocationMemoryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const LocationMemoryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<LocationMemoryRecord> snapshot() const;
 std::vector<LocationMemoryNotice> drainNotices(); void clear();
private:
 LocationMemoryRecord* mutableFind(std::uint64_t); void notice(const LocationMemoryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<LocationMemoryRecord> records_; std::vector<LocationMemoryNotice> notices_;
};
}

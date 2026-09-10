// Intended function: Track security zones, alert levels, authorized groups, lockdown, and breach state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct SecurityZoneSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SecurityZoneSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SecurityZoneSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SecurityZoneSystemSystem {
public:
 bool submit(const SecurityZoneSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SecurityZoneSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SecurityZoneSystemRecord> snapshot() const;
 std::vector<SecurityZoneSystemNotice> drainNotices(); void clear();
private:
 SecurityZoneSystemRecord* mutableFind(std::uint64_t); void notice(const SecurityZoneSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SecurityZoneSystemRecord> records_; std::vector<SecurityZoneSystemNotice> notices_;
};
}

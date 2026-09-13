// Intended function: Track role/capability-based access to colony doors, terminals, machines, ships, and zones.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct AccessControlSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AccessControlSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AccessControlSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AccessControlSystemSystem {
public:
 bool submit(const AccessControlSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AccessControlSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AccessControlSystemRecord> snapshot() const;
 std::vector<AccessControlSystemNotice> drainNotices(); void clear();
private:
 AccessControlSystemRecord* mutableFind(std::uint64_t); void notice(const AccessControlSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AccessControlSystemRecord> records_; std::vector<AccessControlSystemNotice> notices_;
};
}

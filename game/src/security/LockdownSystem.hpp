// Intended function: Coordinate doors, transit, alerts, security forces, and exemptions during lockdowns.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct LockdownSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct LockdownSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct LockdownSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class LockdownSystemSystem {
public:
 bool submit(const LockdownSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const LockdownSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<LockdownSystemRecord> snapshot() const;
 std::vector<LockdownSystemNotice> drainNotices(); void clear();
private:
 LockdownSystemRecord* mutableFind(std::uint64_t); void notice(const LockdownSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<LockdownSystemRecord> records_; std::vector<LockdownSystemNotice> notices_;
};
}

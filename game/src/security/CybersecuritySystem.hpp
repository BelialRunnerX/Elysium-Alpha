// Intended function: Track terminal/network compromise, access escalation, isolation, patches, and recovery.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct CybersecuritySystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CybersecuritySystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CybersecuritySystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CybersecuritySystemSystem {
public:
 bool submit(const CybersecuritySystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CybersecuritySystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CybersecuritySystemRecord> snapshot() const;
 std::vector<CybersecuritySystemNotice> drainNotices(); void clear();
private:
 CybersecuritySystemRecord* mutableFind(std::uint64_t); void notice(const CybersecuritySystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CybersecuritySystemRecord> records_; std::vector<CybersecuritySystemNotice> notices_;
};
}

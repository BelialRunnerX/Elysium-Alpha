// Intended function: Track dynamic cover candidates, integrity, exposure, destruction, and tactical occupancy.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::combatx {
struct CoverSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CoverSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CoverSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CoverSystemSystem {
public:
 bool submit(const CoverSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CoverSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CoverSystemRecord> snapshot() const;
 std::vector<CoverSystemNotice> drainNotices(); void clear();
private:
 CoverSystemRecord* mutableFind(std::uint64_t); void notice(const CoverSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CoverSystemRecord> records_; std::vector<CoverSystemNotice> notices_;
};
}

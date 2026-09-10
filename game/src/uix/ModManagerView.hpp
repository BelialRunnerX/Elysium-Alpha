// Intended function: Project mod packages, dependencies, capabilities, schemas, conflicts, and activation order.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct ModManagerViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ModManagerViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ModManagerViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ModManagerViewSystem {
public:
 bool submit(const ModManagerViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ModManagerViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ModManagerViewRecord> snapshot() const;
 std::vector<ModManagerViewNotice> drainNotices(); void clear();
private:
 ModManagerViewRecord* mutableFind(std::uint64_t); void notice(const ModManagerViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ModManagerViewRecord> records_; std::vector<ModManagerViewNotice> notices_;
};
}

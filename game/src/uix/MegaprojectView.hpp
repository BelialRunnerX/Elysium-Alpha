// Intended function: Project megaproject stages, materials, workforce, logistics, risks, and commissioning.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct MegaprojectViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct MegaprojectViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct MegaprojectViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class MegaprojectViewSystem {
public:
 bool submit(const MegaprojectViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const MegaprojectViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<MegaprojectViewRecord> snapshot() const;
 std::vector<MegaprojectViewNotice> drainNotices(); void clear();
private:
 MegaprojectViewRecord* mutableFind(std::uint64_t); void notice(const MegaprojectViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<MegaprojectViewRecord> records_; std::vector<MegaprojectViewNotice> notices_;
};
}

// Intended function: Project security zones, incidents, patrols, custody, contraband, and lockdown state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct SecurityViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SecurityViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SecurityViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SecurityViewSystem {
public:
 bool submit(const SecurityViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SecurityViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SecurityViewRecord> snapshot() const;
 std::vector<SecurityViewNotice> drainNotices(); void clear();
private:
 SecurityViewRecord* mutableFind(std::uint64_t); void notice(const SecurityViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SecurityViewRecord> records_; std::vector<SecurityViewNotice> notices_;
};
}

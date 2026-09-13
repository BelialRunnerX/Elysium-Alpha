// Intended function: Project system/frame timing, budgets, backlog, percentiles, and hotspots.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct ProfilerViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ProfilerViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ProfilerViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ProfilerViewSystem {
public:
 bool submit(const ProfilerViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ProfilerViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ProfilerViewRecord> snapshot() const;
 std::vector<ProfilerViewNotice> drainNotices(); void clear();
private:
 ProfilerViewRecord* mutableFind(std::uint64_t); void notice(const ProfilerViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ProfilerViewRecord> records_; std::vector<ProfilerViewNotice> notices_;
};
}

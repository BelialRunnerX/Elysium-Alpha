// Intended function: Project timeline, checkpoints, branches, commands, divergence diagnostics, and bookmarks.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct ReplayViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ReplayViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ReplayViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ReplayViewSystem {
public:
 bool submit(const ReplayViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ReplayViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ReplayViewRecord> snapshot() const;
 std::vector<ReplayViewNotice> drainNotices(); void clear();
private:
 ReplayViewRecord* mutableFind(std::uint64_t); void notice(const ReplayViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ReplayViewRecord> records_; std::vector<ReplayViewNotice> notices_;
};
}

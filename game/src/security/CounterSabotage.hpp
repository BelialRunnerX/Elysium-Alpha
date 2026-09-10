// Intended function: Track sabotage risks, investigations, hardening actions, decoys, and discovered plots.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct CounterSabotageRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CounterSabotageRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CounterSabotageNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CounterSabotageSystem {
public:
 bool submit(const CounterSabotageRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CounterSabotageRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CounterSabotageRecord> snapshot() const;
 std::vector<CounterSabotageNotice> drainNotices(); void clear();
private:
 CounterSabotageRecord* mutableFind(std::uint64_t); void notice(const CounterSabotageRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CounterSabotageRecord> records_; std::vector<CounterSabotageNotice> notices_;
};
}

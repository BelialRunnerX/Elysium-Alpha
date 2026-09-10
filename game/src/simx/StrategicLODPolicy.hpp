// Intended function: Choose actor/site simulation fidelity from relevance, proximity, ownership, and unresolved events.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct StrategicLODPolicyRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct StrategicLODPolicyRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct StrategicLODPolicyNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class StrategicLODPolicySystem {
public:
 bool submit(const StrategicLODPolicyRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const StrategicLODPolicyRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<StrategicLODPolicyRecord> snapshot() const;
 std::vector<StrategicLODPolicyNotice> drainNotices(); void clear();
private:
 StrategicLODPolicyRecord* mutableFind(std::uint64_t); void notice(const StrategicLODPolicyRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<StrategicLODPolicyRecord> records_; std::vector<StrategicLODPolicyNotice> notices_;
};
}

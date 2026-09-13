// Intended function: Advance remote settlements/fleets/ecology using aggregate state rather than local actors.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct RemoteAggregateStepperRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct RemoteAggregateStepperRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct RemoteAggregateStepperNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class RemoteAggregateStepperSystem {
public:
 bool submit(const RemoteAggregateStepperRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const RemoteAggregateStepperRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<RemoteAggregateStepperRecord> snapshot() const;
 std::vector<RemoteAggregateStepperNotice> drainNotices(); void clear();
private:
 RemoteAggregateStepperRecord* mutableFind(std::uint64_t); void notice(const RemoteAggregateStepperRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<RemoteAggregateStepperRecord> records_; std::vector<RemoteAggregateStepperNotice> notices_;
};
}

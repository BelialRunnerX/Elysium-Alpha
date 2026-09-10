// Intended function: Track excavation, containment, reactor assembly, thermal routing, and commissioning.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::megaproject {
struct DeepCoreReactorProjectRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct DeepCoreReactorProjectRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct DeepCoreReactorProjectNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class DeepCoreReactorProjectSystem {
public:
 bool submit(const DeepCoreReactorProjectRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const DeepCoreReactorProjectRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<DeepCoreReactorProjectRecord> snapshot() const;
 std::vector<DeepCoreReactorProjectNotice> drainNotices(); void clear();
private:
 DeepCoreReactorProjectRecord* mutableFind(std::uint64_t); void notice(const DeepCoreReactorProjectRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<DeepCoreReactorProjectRecord> records_; std::vector<DeepCoreReactorProjectNotice> notices_;
};
}

// Intended function: Build follow-up missions from consequences and persistent world-state changes.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct DynamicMissionChainRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct DynamicMissionChainRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct DynamicMissionChainNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class DynamicMissionChainSystem {
public:
 bool submit(const DynamicMissionChainRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const DynamicMissionChainRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<DynamicMissionChainRecord> snapshot() const;
 std::vector<DynamicMissionChainNotice> drainNotices(); void clear();
private:
 DynamicMissionChainRecord* mutableFind(std::uint64_t); void notice(const DynamicMissionChainRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<DynamicMissionChainRecord> records_; std::vector<DynamicMissionChainNotice> notices_;
};
}

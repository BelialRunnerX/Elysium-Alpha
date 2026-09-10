// Intended function: Track swarm collector production, launch, orbital distribution, energy routing, and losses.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::megaproject {
struct DysonSwarmProjectRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct DysonSwarmProjectRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct DysonSwarmProjectNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class DysonSwarmProjectSystem {
public:
 bool submit(const DysonSwarmProjectRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const DysonSwarmProjectRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<DysonSwarmProjectRecord> snapshot() const;
 std::vector<DysonSwarmProjectNotice> drainNotices(); void clear();
private:
 DysonSwarmProjectRecord* mutableFind(std::uint64_t); void notice(const DysonSwarmProjectRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<DysonSwarmProjectRecord> records_; std::vector<DysonSwarmProjectNotice> notices_;
};
}

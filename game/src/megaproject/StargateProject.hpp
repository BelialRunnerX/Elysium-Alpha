// Intended function: Track paired gate fabrication, calibration, power reserve, destination lock, and activation state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::megaproject {
struct StargateProjectRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct StargateProjectRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct StargateProjectNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class StargateProjectSystem {
public:
 bool submit(const StargateProjectRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const StargateProjectRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<StargateProjectRecord> snapshot() const;
 std::vector<StargateProjectNotice> drainNotices(); void clear();
private:
 StargateProjectRecord* mutableFind(std::uint64_t); void notice(const StargateProjectRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<StargateProjectRecord> records_; std::vector<StargateProjectNotice> notices_;
};
}

// Intended function: Translate local events into strategic aggregates and strategic events into local intents.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct CrossScaleEventBridgeRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CrossScaleEventBridgeRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CrossScaleEventBridgeNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CrossScaleEventBridgeSystem {
public:
 bool submit(const CrossScaleEventBridgeRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CrossScaleEventBridgeRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CrossScaleEventBridgeRecord> snapshot() const;
 std::vector<CrossScaleEventBridgeNotice> drainNotices(); void clear();
private:
 CrossScaleEventBridgeRecord* mutableFind(std::uint64_t); void notice(const CrossScaleEventBridgeRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CrossScaleEventBridgeRecord> records_; std::vector<CrossScaleEventBridgeNotice> notices_;
};
}

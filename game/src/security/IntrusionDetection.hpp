// Intended function: Track unauthorized access attempts, sensor triggers, confidence, escalation, and response.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct IntrusionDetectionRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct IntrusionDetectionRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct IntrusionDetectionNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class IntrusionDetectionSystem {
public:
 bool submit(const IntrusionDetectionRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const IntrusionDetectionRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<IntrusionDetectionRecord> snapshot() const;
 std::vector<IntrusionDetectionNotice> drainNotices(); void clear();
private:
 IntrusionDetectionRecord* mutableFind(std::uint64_t); void notice(const IntrusionDetectionRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<IntrusionDetectionRecord> records_; std::vector<IntrusionDetectionNotice> notices_;
};
}

// Intended function: Track group-level intent, leader, objective, formation, urgency, and reassignment.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct GroupIntentRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct GroupIntentRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct GroupIntentNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class GroupIntentSystem {
public:
 bool submit(const GroupIntentRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const GroupIntentRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<GroupIntentRecord> snapshot() const;
 std::vector<GroupIntentNotice> drainNotices(); void clear();
private:
 GroupIntentRecord* mutableFind(std::uint64_t); void notice(const GroupIntentRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<GroupIntentRecord> records_; std::vector<GroupIntentNotice> notices_;
};
}

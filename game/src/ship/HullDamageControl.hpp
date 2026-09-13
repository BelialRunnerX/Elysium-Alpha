// Intended function: Track compartment hull damage, isolation, repair priority, patches, and structural recovery.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct HullDamageControlRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct HullDamageControlRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct HullDamageControlNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class HullDamageControlSystem {
public:
 bool submit(const HullDamageControlRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const HullDamageControlRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<HullDamageControlRecord> snapshot() const;
 std::vector<HullDamageControlNotice> drainNotices(); void clear();
private:
 HullDamageControlRecord* mutableFind(std::uint64_t); void notice(const HullDamageControlRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<HullDamageControlRecord> records_; std::vector<HullDamageControlNotice> notices_;
};
}

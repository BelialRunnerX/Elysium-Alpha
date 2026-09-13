// Intended function: Track incoming-fire suppression, morale effects, cover behavior, and recovery.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::combatx {
struct SuppressionSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SuppressionSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SuppressionSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SuppressionSystemSystem {
public:
 bool submit(const SuppressionSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SuppressionSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SuppressionSystemRecord> snapshot() const;
 std::vector<SuppressionSystemNotice> drainNotices(); void clear();
private:
 SuppressionSystemRecord* mutableFind(std::uint64_t); void notice(const SuppressionSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SuppressionSystemRecord> records_; std::vector<SuppressionSystemNotice> notices_;
};
}
